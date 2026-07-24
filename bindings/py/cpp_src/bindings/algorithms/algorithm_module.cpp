/* ---------------------------------------------------------------------
 * HTM Community Edition of NuPIC
 * Copyright (C) 2018, Numenta, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Affero Public License for more details.
 *
 * You should have received a copy of the GNU Affero Public License
 * along with this program.  If not, see http://www.gnu.org/licenses.
 *
 * Author: @chhenning, 2018
 * --------------------------------------------------------------------- */

/** @file
Algorithm bindings Module file for pybind11
*/

#include <bindings/suppress_register.hpp>  //must be before pybind11.h
#include <pybind11/pybind11.h>

#include <htm/algorithms/Anomaly.hpp>
#include <htm/algorithms/SpatialPooler.hpp>
#include <htm/algorithms/TemporalMemory.hpp>
#include <htm/types/Sdr.hpp>

namespace py = pybind11;

namespace htm_ext
{
    void init_Connections(py::module&);
    void init_TemporalMemory(py::module&);
    void init_Spatial_Pooler(py::module&);

} // namespace htm_ext

using namespace htm_ext;
using htm::SDR;
using htm::SpatialPooler;
using htm::TemporalMemory;
using htm::UInt;

PYBIND11_MODULE(algorithms, m) {
    m.doc() = "htm.core.algorithms plugin"; // optional module docstring

    init_Connections(m);
    init_TemporalMemory(m);
    init_Spatial_Pooler(m);

    // Module-level anomaly helper, exposed for PyHTM: its Python-side
    // calc_anomaly_score() re-implemented exactly this formula by allocating
    // an SDR, intersecting, and counting -- per step, per module, under the
    // GIL. This is the core's own computeRawAnomalyScore, one C++ call:
    //     1 - |active AND predicted| / |active|      (0 when active is empty)
    m.def("computeRawAnomalyScore",
        [](const SDR &active, const SDR &predicted) {
            py::gil_scoped_release release;
            return htm::computeRawAnomalyScore(active, predicted);
        },
        py::arg("activeColumns"), py::arg("predictedColumns"),
R"(Computes the raw anomaly score between two SDRs of the same dimensions:
1 - |active AND predicted| / |active|.  Returns 0.0 when `activeColumns` has
no active bits.  Both arguments are column-level SDRs (use
TM.cellsToColumns() to convert cell SDRs).)");

    // ---------------------------------------------------------------------
    // stepSpTm: PyHTM's whole per-module forward step as ONE C++ call under
    // ONE GIL release. The classic Python path crosses the Python<->C++
    // boundary 6-7 times per step (SP.compute, activateDendrites,
    // getPredictiveCells, cellsToColumns, anomaly, activateCells,
    // getActiveCells/residual); every crossing pays a GIL release+reacquire
    // plus Python object wrapping. Here the SAME functions run in the SAME
    // order inside one release -- so results are identical by construction
    // (proven bit-exact per step by PyHTM's seam test T6), and the per-step
    // GIL traffic collapses to a single acquire/release pair.
    //
    // residualMode: 0 = return the active cells (anomaly-detector off, or
    //                   residual_strength <= 0 which forwards the full
    //                   active signal);
    //               1 = 'subtract' residual (active AND NOT predicted);
    //               2 = 'xor' residual ((a-p) OR (p-a)).
    // The experimental 0 < residual_strength < 1 sampling stays on the
    // classic Python path (it draws from numpy's global RNG).
    // `sp` may be None (encoder-only modules): the input SDR then plays the
    // active-columns role directly, exactly like the Python path.
    m.def("stepSpTm",
        [](SpatialPooler *sp, TemporalMemory &tm, const SDR &input,
           const bool learn, const bool calcAnomaly, const int residualMode,
           SDR *externalActive, SDR *externalWinning) {
            NTA_CHECK(residualMode >= 0 && residualMode <= 2)
                << "residualMode must be 0 (off), 1 (subtract) or 2 (xor).";
            double anomaly = 0.0;
            size_t nActiveColumns = 0, nPredictiveCells = 0;
            SDR *out = nullptr;
            {
                py::gil_scoped_release release;

                // --- Spatial Pooler (or encoder pass-through) ---
                SDR activeLocal;
                const SDR *activeColumns;
                if (sp != nullptr) {
                    activeLocal.initialize(sp->getColumnDimensions());
                    sp->compute(input, learn, activeLocal);
                    activeColumns = &activeLocal;
                } else {
                    activeColumns = &input;   // module without an SP
                }
                nActiveColumns = activeColumns->getSparse().size();

                // --- Temporal Memory: predict, score, then activate ---
                // ORDER IS THE ALGORITHM (same as the Python path): dendrites
                // predict from t-1 cells; anomaly compares those predictions
                // to this step's columns; only then are cells activated.
                if (externalActive != nullptr && externalWinning != nullptr) {
                    tm.activateDendrites(learn, *externalActive, *externalWinning);
                } else {
                    tm.activateDendrites(learn);
                }
                SDR predictiveCells = tm.getPredictiveCells();
                nPredictiveCells = predictiveCells.getSparse().size();
                if (calcAnomaly) {
                    const SDR predictiveColumns = tm.cellsToColumns(predictiveCells);
                    anomaly = htm::computeRawAnomalyScore(*activeColumns, predictiveColumns);
                }
                tm.activateCells(*activeColumns, learn);

                // --- Output: active cells, or the RTM residual ---
                auto cellDims = tm.getColumnDimensions();
                cellDims.push_back(static_cast<UInt>(tm.getCellsPerColumn()));
                out = new SDR(cellDims);
                if (residualMode == 0) {
                    tm.getActiveCells(*out);
                } else {
                    SDR activeCells(cellDims);
                    tm.getActiveCells(activeCells);
                    if (residualMode == 1) {
                        out->subtract(activeCells, predictiveCells);
                    } else {
                        SDR t1(cellDims), t2(cellDims);
                        t1.subtract(activeCells, predictiveCells);
                        t2.subtract(predictiveCells, activeCells);
                        out->set_union(t1, t2);
                    }
                }
            }
            return py::make_tuple(
                calcAnomaly ? py::object(py::float_(anomaly)) : py::object(py::none()),
                nActiveColumns, nPredictiveCells,
                py::cast(out, py::return_value_policy::take_ownership));
        },
        py::arg("sp").none(true), py::arg("tm"), py::arg("input"),
        py::arg("learn"), py::arg("calcAnomaly"), py::arg("residualMode"),
        py::arg("externalActive").none(true) = nullptr,
        py::arg("externalWinning").none(true) = nullptr,
R"(One full PyHTM module step (SP -> TM -> anomaly -> residual) executed as a
single C++ call under a single GIL release. Returns a tuple:
(anomaly_or_None, num_active_columns, num_predictive_cells, output_SDR).
`sp` may be None for encoder-only modules; residualMode: 0=active cells,
1=subtract residual, 2=xor residual. Identical, call-for-call, to the
classic per-function path.)");
}

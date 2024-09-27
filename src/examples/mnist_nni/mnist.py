""" An MNIST classifier using Spatial Pooler."""

import argparse
import logging
import math
import random
import gzip
import numpy as np
import os
import threading
from pprint import pprint

from nupic.bindings.algorithms import SpatialPooler, Classifier
from nupic.bindings.sdr import SDR, Metrics

import nni


def load_mnist(path):
    """See: http://yann.lecun.com/exdb/mnist/ for MNIST download and binary file format spec."""
    def int32(b):
        i = 0
        for char in b:
            i *= 256
            # i += ord(char)    # python2
            i += char
        return i

    def load_labels(file_name):
        with gzip.open(file_name, 'rb') as f:
            raw = f.read()
            assert(int32(raw[0:4]) == 2049)  # Magic number
            labels = []
            for char in raw[8:]:
                # labels.append(ord(char))      # python2
                labels.append(char)
        return labels

    def load_images(file_name):
        with gzip.open(file_name, 'rb') as f:
            raw = f.read()
            assert(int32(raw[0:4]) == 2051)    # Magic number
            num_imgs   = int32(raw[4:8])
            rows       = int32(raw[8:12])
            cols       = int32(raw[12:16])
            assert(rows == 28)
            assert(cols == 28)
            img_size   = rows*cols
            data_start = 4*4
            imgs = []
            for img_index in range(num_imgs):
                vec = raw[data_start + img_index*img_size : data_start + (img_index+1)*img_size]
                # vec = [ord(c) for c in vec]   # python2
                vec = list(vec)
                vec = np.array(vec, dtype=np.uint8)
                buf = np.reshape(vec, (rows, cols, 1))
                imgs.append(buf)
            assert(len(raw) == data_start + img_size * num_imgs)   # All data should be used.
        return imgs

    train_labels = load_labels(os.path.join(path, 'train-labels-idx1-ubyte.gz'))
    train_images = load_images(os.path.join(path, 'train-images-idx3-ubyte.gz'))
    test_labels  = load_labels(os.path.join(path, 't10k-labels-idx1-ubyte.gz'))
    test_images  = load_images(os.path.join(path, 't10k-images-idx3-ubyte.gz'))

    return train_labels, train_images, test_labels, test_images


class BWImageEncoder:
    """Simple grey scale image encoder for MNIST."""
    def __init__(self, input_space):
        self.output = SDR(tuple(input_space))

    def encode(self, image):
        self.output.dense = image >= np.mean(image)
        return self.output


def main(args):
    # Load data.
    train_labels, train_images, test_labels, test_images = load_mnist(args['data_dir'])
    training_data = list(zip(train_images, train_labels))
    test_data     = list(zip(test_images, test_labels))
    random.shuffle(training_data)
    random.shuffle(test_data)

    # Setup the AI.
    enc = BWImageEncoder(train_images[0].shape[:2])
    sp = SpatialPooler(
        inputDimensions            = enc.output.dimensions,
        columnDimensions           = [int(args['columnDimensions']), 1],
        potentialRadius            = 99999999,
        potentialPct               = args['potentialPct'],
        globalInhibition           = True,
        localAreaDensity           = args['localAreaDensity'],
        numActiveColumnsPerInhArea = -1,
        stimulusThreshold          = int(round(args['stimulusThreshold'])),
        synPermInactiveDec         = args['synPermInactiveDec'],
        synPermActiveInc           = args['synPermActiveInc'],
        synPermConnected           = args['synPermConnected'],
        minPctOverlapDutyCycle     = args['minPctOverlapDutyCycle'],
        dutyCyclePeriod            = int(round(args['dutyCyclePeriod'])),
        boostStrength              = args['boostStrength'],
        seed                       = 42,
        spVerbosity                = 99,
        wrapAround                 = False)
    columns = SDR( sp.getColumnDimensions() )
    columns_stats = Metrics( columns, 99999999 )
    sdrc = Classifier()

    # Training Loop
    for i in range(len(train_images)):
        img, lbl = random.choice(training_data)
        enc.encode(np.squeeze(img))
        sp.compute( enc.output, True, columns )
        sdrc.learn( columns, lbl )

    print(str(sp))
    print(str(columns_stats))

    # Testing Loop
    score = 0
    for img, lbl in test_data:
        enc.encode(np.squeeze(img))
        sp.compute( enc.output, False, columns )
        if lbl == np.argmax( sdrc.infer( columns ) ):
            score += 1

    print('Score:', 100 * score / len(test_data), '%')
    nni.report_final_result( score / len(test_data) )


def get_params():
    ''' Get parameters from command line '''
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_dir", type=str, default="./MNIST_data")

    parser.add_argument("--columnDimensions",       type=int,   default = 10000)
    parser.add_argument("--potentialPct",           type=float, default = 0.5)
    parser.add_argument("--localAreaDensity",       type=float, default = .015)
    parser.add_argument("--stimulusThreshold",      type=int,   default = 6)
    parser.add_argument("--synPermActiveInc",       type=float, default = 0.01)
    parser.add_argument("--synPermInactiveDec",     type=float, default = 0.005)
    parser.add_argument("--synPermConnected",       type=float, default = 0.4)
    parser.add_argument("--minPctOverlapDutyCycle", type=float, default = 0.001)
    parser.add_argument("--dutyCyclePeriod",        type=int,   default = 1000)
    parser.add_argument("--boostStrength",          type=float, default = 2.5)

    args, _ = parser.parse_known_args()
    return args

if __name__ == '__main__':
    params = vars(get_params())
    tuner_params = nni.get_next_parameter()
    if tuner_params is not None:
        params.update(tuner_params)
    pprint(params)

    timer = threading.Timer( 30 * 60, lambda: os._exit(1))
    timer.daemon = True
    timer.start()
    main(params)
    timer.cancel()

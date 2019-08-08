# ------------------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2016, Numenta, Inc. https://numenta.com
#               2019, David McDougall
#               2019, Brev Patterson, Lux Rota LLC, https://luxrota.com
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU Affero Public License version 3 as published by
# the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero Public License for
# more details.
#
# You should have received a copy of the GNU Affero Public License along with
# this program.  If not, see http://www.gnu.org/licenses.
# ------------------------------------------------------------------------------

import htm.bindings.encoders
SimHashDocumentEncoder = htm.bindings.encoders.SimHashDocumentEncoder
SimHashDocumentEncoderParameters = htm.bindings.encoders.SimHashDocumentEncoderParameters

__all__ = ['SimHashDocumentEncoder', 'SimHashDocumentEncoderParameters']


if __name__ == '__main__':
  """
  Simple program to examine the SimHashDocumentEncoder.

  For help using this program run:
  $ python -m htm.encoders.simhash_document_encoder --help
  """
  import argparse
  import matplotlib.pyplot as plot
  import numpy
  import random
  import textwrap

  from sys import exit

  from htm.bindings.sdr import SDR, Metrics


  # Gather input from the user.
  parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description="Simple program to examine the SimHashDocumentEncoder.\n\n" +
    textwrap.dedent(SimHashDocumentEncoder.__doc__ + "\n\n" +
      SimHashDocumentEncoderParameters.__doc__))
  parser.add_argument('--activeBits', type=int, default=0,
    help=SimHashDocumentEncoderParameters.activeBits.__doc__)
  parser.add_argument('--size', type=int, default=0,
    help=SimHashDocumentEncoderParameters.size.__doc__)
  parser.add_argument('--sparsity', type=float, default=0,
    help=SimHashDocumentEncoderParameters.sparsity.__doc__)
  parser.add_argument('--tokenSimilarity', action='store_true', default=False,
    help=SimHashDocumentEncoderParameters.tokenSimilarity.__doc__)
  args = parser.parse_args()

  # Copy the command line arguments into the parameter structure.
  parameters = SimHashDocumentEncoderParameters()
  parameters.activeBits = args.activeBits
  parameters.size = args.size
  parameters.sparsity = args.sparsity
  parameters.tokenSimilarity = args.tokenSimilarity

  # Try initializing the encoder.
  try:
    encoder = SimHashDocumentEncoder(parameters)
  except RuntimeError as error:
    print(error)
    parser.print_usage()
    exit()

  # Run the encoder and measure some statistics about its output.
  num_samples = 1000 # number of documents to run
  num_tokens = 10 # tokens per document
  testCorpus = [ "find", "any", "new", "work", "part", "take", "get", "place",
    "made", "live", "where", "after", "back", "little", "only", "round", "man",
    "year", "came", "show", "every", "good", "me", "give", "our", "under",
    "name", "very", "through", "just", "form", "sentence", "great", "think",
    "say", "help", "low", "line", "differ", "turn", "cause", "much", "mean",
    "before", "move", "right", "boy", "old", "too", "same", "tell", "does",
    "set", "three", "want", "air", "well", "also", "play", "small", "end",
    "put", "home", "read", "hand", "port", "large", "spell", "add", "even",
    "land", "here", "must", "big", "high", "such", "follow", "act", "why",
    "ask", "men", "change", "went", "light", "kind", "off", "need", "house",
    "picture", "try", "us", "again", "animal", "point", "mother", "world",
    "near", "build", "self", "earth" ] # 100 simple common english words
  documents = []
  sdrs = []
  reference = {} # reference document to compare against for similarity checks
  similar   = {} # most similar document to the reference
  unsimilar = {} # least similar document against reference
  distance = lambda a, b : numpy.count_nonzero(a != b)

  for _ in range(num_samples):
    document = []
    for _ in range(num_tokens - 1):
      document.append(testCorpus[random.randint(0, len(testCorpus) - 1)])
    document.sort()
    documents.append(document)
    sdr = encoder.encode(document)
    sdrs.append(sdr)

    # similarity checking
    current = numpy.zeros([encoder.size], dtype=numpy.uint8)
    current[:] = sdr.dense

    if not reference:
      reference = { "doc": document, "bits": current }
    else:
      if not similar:
        similar   = { "doc": document, "bits": current }
      else:
        if (distance(current, reference["bits"]) <
            distance(similar["bits"], reference["bits"])):
          similar = { "doc": document, "bits": current }

      if not unsimilar:
        unsimilar = { "doc": document, "bits": current }
      else:
        if (distance(current, reference["bits"]) >
            distance(unsimilar["bits"], reference["bits"])):
          unsimilar = { "doc": document, "bits": current }

  report = Metrics([encoder.size], len(sdrs) + 1)
  for sdr in sdrs:
    report.addData(sdr)

  print("Statistics:")
  print("\tEncoded %d Document inputs." % len(sdrs))
  print("\tOutput: " + str(report))

  print("Similarity:")
  print("\tReference:\n\t\t" + str(reference["doc"]))
  print("\tMOST Similar (Distance = " +
    str(distance(similar["bits"], reference["bits"])) + "):")
  print("\t\t" + str(similar["doc"]))
  print("\tLEAST Similar (Distance = " +
    str(distance(unsimilar["bits"], reference["bits"])) + "):")
  print("\t\t" + str(unsimilar["doc"]))

  # Plot the Receptive Field of each bit in the encoder.
  field = numpy.zeros([encoder.size, len(sdrs)], dtype=numpy.uint8)
  for i in range(len(sdrs)):
    field[:, i] = sdrs[i].dense
  plot.imshow(field, interpolation='nearest')
  plot.title("SimHash Document Encoder - Receptive Fields")
  plot.xlabel("Input Document #")
  plot.ylabel("SDR Bit #")
  plot.show()
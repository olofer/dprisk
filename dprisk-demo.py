#
# EXAMPLES:
#   python dprisk-demo.py
#   python dprisk-demo.py --help
#   python dprisk-demo.py --attackers 128 --defenders 128 --samples 1000000 --pdf --png
#

import os
import argparse
import numpy as np
import matplotlib.pyplot as plt

if __name__ == '__main__':

  parser = argparse.ArgumentParser()

  parser.add_argument('--attackers', type = int, default = 70, help = 'number of army units (attacker)')
  parser.add_argument('--defenders', type = int, default = 75, help = 'number of army units (defender)')
  parser.add_argument('--samples', type = int, default = 10000, help = 'number of samples (for simulation)')
  parser.add_argument('--pdf', action = 'store_true', help = 'save PDF figure of DP solution')
  parser.add_argument('--png', action = 'store_true', help = 'save PNG figure of DP solution')
  parser.add_argument('--png-dpi', type = float, default = 150.0, help = 'dpi for PNG file')

  args = parser.parse_args()

  print('A = {}, D = {}'.format(args.attackers, args.defenders))

  tablefilename = 'dprisk-demo-{}x{}.txt'.format(args.attackers, args.defenders)
  commandstring = './dprisk {} {} > {}'.format(args.attackers, args.defenders, tablefilename)

  print('call: {}'.format(commandstring))
  retcode_dp = os.system(commandstring)

  if retcode_dp == 0:
    print('loading: {}'.format(tablefilename))

    M = np.loadtxt(tablefilename)
    assert M.shape[0] == args.attackers + 1
    assert M.shape[1] == args.defenders + 1

    print('Prob[A wins](A = {}, D = {}) = {:.6f}'.format(args.attackers, args.defenders, M[-1, -1]))

    if args.pdf or args.png:
      figfilename = 'dprisk-demo-{}x{}'.format(args.attackers, args.defenders)
    
      plt.imshow(M)
      plt.colorbar()
      plt.xlabel('number of defender units')
      plt.ylabel('number of attacker units')
      plt.title('Probability of attacker winning battle')
      plt.tight_layout()

      if args.pdf:
        pdfname = figfilename + '.pdf'
        print('writing: {}'.format(pdfname))
        plt.savefig(pdfname)

      if args.png:
        pngname = figfilename + '.png'
        print('writing: {}'.format(pngname))
        plt.savefig(pngname, dpi = args.png_dpi)

  if args.samples > 0:
    print('sampling ({} simulations)'.format(args.samples))
    os.system('./dprisk {} {} {}'.format(args.attackers, args.defenders, args.samples))

  print('done.')

#!/usr/bin/env python3

import sys

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: %s [MONITOR_PREFIX_NAME] [NUMBER_OF_ISLAND]" % sys.argv[0])
        sys.exit()

    prefix = sys.argv[1]
    N = int(sys.argv[2])

    files = []
    for i in range(N):
        files += [open("%s_monitor_%d" % (prefix, i), 'r')]

    newfile = open("%s_monitor" % prefix, 'w')

    newfile.write("""# Command line : ../OneMax -B 1000 --seed 1 --Ver 2 --Ind 25 --Mig 10000 --Bet 0.99 --Alp 0.8 --MvLS 1 --IdG 100 --InitG 1 --Isl 4 --LSF ../../OpFiles/LSop4.txt --Upd 0 --GB 1
# Instance 1000 loaded
# LS operators for islands;bit flip;1 flip;3 flips;5 flips;
# col 1: migration
# col 2: nb_individual_isl0
# col 3: avg_ones_isl0
# col 4: delta_avg_ones_isl0
# col 5: best_value_isl0
# col 6: nb_input_ind_isl0
# col 7: nb_output_ind_isl0
# col 8: P0to0
# col 9: P0to1
# col 10: P0to2
# col 11: P0to3
# col 12: P*to0
# col 13: nb_migrants_isl0to0
# col 14: nb_migrants_isl0to1
# col 15: nb_migrants_isl0to2
# col 16: nb_migrants_isl0to3
# col 17: nb_individual_isl1
# col 18: avg_ones_isl1
# col 19: delta_avg_ones_isl1
# col 20: best_value_isl1
# col 21: nb_input_ind_isl1
# col 22: nb_output_ind_isl1
# col 23: P1to0
# col 24: P1to1
# col 25: P1to2
# col 26: P1to3
# col 27: P*to1
# col 28: nb_migrants_isl1to0
# col 29: nb_migrants_isl1to1
# col 30: nb_migrants_isl1to2
# col 31: nb_migrants_isl1to3
# col 32: nb_individual_isl2
# col 33: avg_ones_isl2
# col 34: delta_avg_ones_isl2
# col 35: best_value_isl2
# col 36: nb_input_ind_isl2
# col 37: nb_output_ind_isl2
# col 38: P2to0
# col 39: P2to1
# col 40: P2to2
# col 41: P2to3
# col 42: P*to2
# col 43: nb_migrants_isl2to0
# col 44: nb_migrants_isl2to1
# col 45: nb_migrants_isl2to2
# col 46: nb_migrants_isl2to3
# col 47: nb_individual_isl3
# col 48: avg_ones_isl3
# col 49: delta_avg_ones_isl3
# col 50: best_value_isl3
# col 51: nb_input_ind_isl3
# col 52: nb_output_ind_isl3
# col 53: P3to0
# col 54: P3to1
# col 55: P3to2
# col 56: P3to3
# col 57: P*to3
# col 58: nb_migrants_isl3to0
# col 59: nb_migrants_isl3to1
# col 60: nb_migrants_isl3to2
# col 61: nb_migrants_isl3to3
# col 62: global best value4
# col 63: global avg ones4
# col 64: number of evaluations4

""")

    # on ignore la premiere ligne
    for i in range(N):
        files[i].readline()

    for line in files[0]:
        t0 = line.split()
        newfile.write('%s ' % ' '.join(t0[2:]))
        best = []
        nbindi = []
        avg = []
        evl = []
        best += [float(t0[6])]
        nbindi += [float(t0[3])]
        avg += [float(t0[4]) * float(t0[3])]
        for i in range(1,4):
            ti = files[i].readline().split()
            newfile.write('%s ' % ' '.join(ti[3:]))
            best += [float(ti[6])]
            nbindi += [float(ti[3])]
            avg += [float(ti[4]) * float(ti[3])]
        newfile.write('%d %d %d' % (max(best), sum(avg) / sum(nbindi), 0))
        newfile.write('\n')

    print("Done")

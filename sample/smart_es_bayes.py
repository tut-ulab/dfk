# -*- coding: utf-8 -*-

'''
input:
paper_id, abstract


'''

import sys
import dfk


def bigram(text):
    for i in range(len(text)-1):
        yield text[i:i+2]


def parse_args():
    import argparse
    parser = argparse.ArgumentParser(
        description='Usage: python %s selected_space.class domain_space.class < text.dat' % sys.argv[0])
    parser.add_argument('selected_space', help='')
    parser.add_argument('domain_space', help='')
    parser.add_argument('--input', default=sys.stdin, type=argparse.FileType('r'), help='Input text (default: stdin)')
    return parser.parse_args()


if __name__ == '__main__':
    args = parse_args()

    select_space = dfk.open(args.selected_space)
    domain_space = dfk.open(args.domain_space)

    for line in args.input:
        paper_id, abst = line.rstrip().split('\t')

        for start, bi in enumerate(bigram(abst)):
            df_select = select_space.dfn(1, bi)
            df_domain = domain_space.dfn(1, bi)
            r = df_select / (df_domain + 0.5)
            print(paper_id, start, 2, bi, r, '-', sep='\t')

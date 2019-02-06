# -*- coding: utf-8 -*-

'''
input:
paper_id, abstract


'''

import sys
import _dfk


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

    select_space = _dfk.setup(args.selected_space)
    domain_space = _dfk.setup(args.domain_space)

    for line in args.input:
        paper_id, abst = line.rstrip().split('\t')

        for start, bi in enumerate(bigram(abst)):
            _dfk.use(select_space)
            df_select = _dfk.dfn(1, bi)
            _dfk.use(domain_space)
            df_domain = _dfk.dfn(1, bi)
            r = df_select / (df_domain + 0.5)
            print(paper_id, start, 2, bi, r, '-', sep='\t')

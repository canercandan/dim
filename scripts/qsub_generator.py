#!/usr/bin/env python3

from parser import Parser
import sys

TEMPLATE = """\
#!/bin/bash
##
## automagically generated thanks to the command line:
## %(generator_cmd)s
##
#$ -N %(name)s
#$ -e %(error_file)s
#$ -o %(out_file)s
#$ -j %(error_in_out_file)s
%(current_directory)s
%(current_environment)s
#$ -S %(shell)s
#$ -p %(priority)d
#$ -l mem_free=%(mem_free)s
#$ -l h_vmem=%(h_vmem)s
#$ -m %(notification)s
#$ -pe threaded %(nb_threads)d
%(array)s

NISLANDS=%(nislands)d
POPSIZE=%(popsize)d
GENMAX=%(genmax)d
INSTANCE="%(instance)s"
PROBASAME="%(proba_same)s"
OPERATORS="%(operators)s"
ALPHA=%(alpha)s
BETA=%(beta)s
NBMOVE=%(nbmove)d

CMD="%(cmd_path)s --status=$JOB_ID/tsp.status --monitorPrefix=$JOB_ID/result_$SGE_TASK_ID -N=$NISLANDS -P=$POPSIZE $INSTANCE -G=$GENMAX $PROBASAME -a=$ALPHA -b=$BETA $OPERATORS --nbmove=$NBMOVE"
echo "$NSLOTS $JOB_ID $HOSTNAME" && mkdir $JOB_ID && echo $CMD > $JOB_ID/CMD && echo $CMD && $CMD && exit 0\
"""

DEFAULT_CONFIG = {
    'generator_cmd': '',
    'name': 'test',
    'error_file': 'err.dat',
    'out_file': 'out.dat',
    'error_in_out_file': True,
    'current_directory': True,
    'current_environment': True,
    'shell': '/bin/bash',
    'priority': 0,
    'mem_free': '1G',
    'h_vmem': '2G',
    'notification': 'n',
    'nb_threads': 1,
    'cmd_path': '',
    'nislands': 12,
    'popsize': 100,
    'genmax': 10000,
    'proba_same': None,
    'alpha': 0.2,
    'beta': 0.01,
    'nbmove': 1,
    'array': None,
}

def generate(**kwargs):
    config = {}
    config.update(DEFAULT_CONFIG)
    config.update(kwargs)

    config['error_in_out_file'] = 'y' if config.get('error_in_out_file', True) else 'n'
    config['current_directory'] = '#$ -cwd' if config.get('current_directory', True) else ''
    config['current_environment'] = '#$ -V' if config.get('current_environment', True) else ''
    config['array'] = '#$ -t 1-%(array)d' % config if config.get('array', None) else ''
    config['proba_same'] = '-d=%(proba_same)s' % config if config.get('proba_same', None) else ''
    config['instance'] = '--tspInstance=%(instance)s' % config if config.get('instance', None) else ''
    config['operators'] = '--operators=%(operators)s' % config if config.get('operators', None) else ''
    config['generator_cmd'] = ' '.join(sys.argv)

    return TEMPLATE % config

parser = Parser(description='qsub script generator.', verbose='error')
parser.add_argument('--name', '-n', help='give a name', default='test')
parser.add_argument('--nislands', '-N', help='number of islands', type=int, default=4)
parser.add_argument('--popsize', '-P', help='size of population', type=int, default=100)
parser.add_argument('--genmax', '-G', help='maximum number of generation (0: disable)', type=int, default=0)
parser.add_argument('--alpha', '-a', help='the alpha parameter of the learning process', type=float, default=.2)
parser.add_argument('--beta', '-b', help='the beta parameter of the learning process', type=float, default=.01)
parser.add_argument('--proba_same', '-D', help='proba same', type=float, default=0)
parser.add_argument('--instance', '-I', help='tsp instance', default='')
parser.add_argument('--operators', '-O', help='list of operators separated by comma', default='')
parser.add_argument('--cmd_path', '-C', help='cmd path', default='./tsp')
parser.add_argument('--nb_threads', '-T', help='number of threads', type=int, default=1)
parser.add_argument('--nbmove', '-m', help='number of movements', type=int, default=1)
parser.add_argument('--array', '-A', help='number of jobs in job array', type=int, default=0)
args = parser()

# print(args[0])

print(generate(**args.__dict__))

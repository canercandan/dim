#!/usr/bin/env python3

from parser import Parser

TEMPLATE = """\
#!/bin/bash
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

echo "$NSLOTS $JOB_ID $HOSTNAME"
CMD="%(cmd_path)s -N=%(nislands)d -P=%(popsize)d %(instance)s -G=%(genmax)d %(proba_same)s -a=%(alpha)s -b=%(beta)s %(operators)s --nbmove=%(nbmove)d"
echo $CMD > CMD && echo $CMD && $CMD
exit 0\
"""

DEFAULT_CONFIG = {
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
}

def generate(**kwargs):
    config = {}
    config.update(DEFAULT_CONFIG)
    config.update(kwargs)

    config['error_in_out_file'] = 'y' if config.get('error_in_out_file', True) else 'n'
    config['current_directory'] = '#$ -cwd' if config.get('current_directory', True) else ''
    config['current_environment'] = '#$ -V' if config.get('current_environment', True) else ''
    config['proba_same'] = '-d=%(proba_same)s' % config if 'proba_same' in config and config['proba_same'] else ''
    config['instance'] = '--tspInstance=%(instance)s' % config if 'instance' and config['instance'] in config else ''
    config['operators'] = '--operators=%(operators)s' % config if 'operators' in config and config['operators'] else ''

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
args = parser()

print(generate(**args.__dict__))

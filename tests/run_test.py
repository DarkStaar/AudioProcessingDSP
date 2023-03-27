import itertools
import os
import pathlib
import re
import shutil
import subprocess
import wave
import xml.etree.ElementTree as ET

# input_streams_root == "/path/to/script/../../Input/Teststreams"
input_streams_root = os.path.join(os.path.dirname(__file__), "TestStreams")
# base name of project
target = "delay"
# executables_root = "/path/to/script/Debug"
executables_root = os.path.join(os.path.dirname(__file__), "bin")
# compare_tool = "/path/to/script/Tools/PCMCompare.exe"
compare_tool = os.path.join(os.path.dirname(__file__), "Tools", "PCMCompare.exe")

# SET CIRRUS DSP SDK ROOT
clide_root = "C:\\CirrusDSP"

# name of models for testing == ["model0", "model1", "model2"]
models = [f'model{x}' for x in range(3)]
# *************************************************************************
# This tool expects the executables provited to be of the following format:
#   executables = [ f'{target}_{m}' for m in models ]
#   i.e. multitapEcho_model0
# *************************************************************************


# returns list of paths to input streams
def get_inputs():
    # Get all .wav files from input_streams_root
    # inputs = []
    # for root, dirs, files in os.walk(input_streams_root):
    #     for file in files:
    #         if file.endswith(".wav"):
    #             inputs.append(os.path.join(root, file))

    # inputs == ["/path/to/input_streams/funky_sample.wav", "/path/to/input_streams/WhiteNoise.wav"]
    inputs = [os.path.join(input_streams_root, x) for x in ['speech_2ch.wav', 'titanic_horn.wav', 'funky_sample.wav']]
    
    return inputs
    
# returns list of lists of desired params
def get_params():
    
    params = [
        [0 , 1, 2],
        [1],
    ]

    return params

################################################################################
### DO NOT EDIT BELOW THIS LINE
################################################################################

# test_outputs = "/path/to/script/test_outputs"
test_outputs = os.path.join(os.path.dirname(__file__), "test_outputs")
exe_log_file = open(os.path.join(os.path.dirname(__file__), 'test_execution_log.txt'), 'w')

def simple_log(str):
    print(str)
    exe_log_file.write(str + '\n')


def get_cases():
    cases = []
    params = get_params()
    combinations = list(itertools.product(*params))

    cases = list(itertools.product(get_inputs(), combinations))

    return cases


def run_compare(in_file, out_file, log_file):
    sampwidth = wave.open(in_file, 'rb').getsampwidth()

    cmd = f'"{compare_tool}" "{in_file}" "{out_file}" -b{8*sampwidth}'
    simple_log(cmd)
    log_file.write(cmd + "\n")

    proc_data = subprocess.run(cmd, capture_output=True, shell=True)

    log_file.write(proc_data.stderr.decode() + "\n")
    log_file.write(proc_data.stdout.decode() + "\n")



def execute_cmd(cmd):
    proc_data = subprocess.run(cmd, capture_output=True, shell=True)
    
    simple_log(proc_data.stdout.decode())
    simple_log(proc_data.stderr.decode())


def execute_test(exe, in_file, out_file, add_args):
    add_params = " ".join(str(arg) for arg in add_args)

    cmd = f'"{exe}" "{in_file}" "{out_file}" {add_params}'
    simple_log(cmd)
    execute_cmd(cmd)

def to_xml(sim_argv, uld):
    cl_proj = ET.Element('CL_PROJECT')
    #argv
    argv = ET.SubElement(cl_proj, 'argv')
    argv.text = sim_argv

    # FILE_IO_CFG
    in_cfg_attrs = {
        'block_type': 'Input',
        'channels_per_line': '2',
        'delay': '0',
        'file_mode': 'PCM',
        'index': '0',
        'justification': 'Left',
        'sample_format': 'LittleEndian',
        'sample_rate': '48000',
        'sample_size': '32'
    }
    ET.SubElement(cl_proj, 'FILE_IO_CFG', in_cfg_attrs)

    #FILE_IO_CFG
    out_cfg_attrs = {
        'block_type': 'Output',
        'channels_per_line': '2',
        'delay': '0',
        'file_mode': 'PCM',
        'index': '0',
        'justification': 'Left',
        'sample_format': 'LittleEndian',
        'sample_rate': '48000',
        'sample_size': '32'
    }
    ET.SubElement(cl_proj, 'FILE_IO_CFG', out_cfg_attrs)

    #MEMORY_CFG
    mem_cfg = ET.SubElement(cl_proj, 'MEMORY_CFG')
    #ULD_FILE
    uld_file_attrs = {
        'disk_path': uld
    }
    ET.SubElement(mem_cfg, 'ULD_FILE', uld_file_attrs)

    #PROFILE_CFG
    ET.SubElement(cl_proj, 'PROFILE_CFG', {'enable':'on'})

    #SCP_CFG
    ET.SubElement(cl_proj, 'SCP_CFG', {'load_delay':'on'})

    xml_file = os.path.join(os.path.dirname(__file__), 'temp.sbr')
    tree = ET.ElementTree(cl_proj)
    ET.indent(tree, '    ')
    tree.write(xml_file, encoding='UTF-8', xml_declaration=True)

    return xml_file


def model3_magic(uld, in_file, out_file, add_args):
    add_params = " ".join(str(arg) for arg in add_args)
    u = f'{pathlib.Path(uld).as_posix()}'
    i = f'{pathlib.Path(in_file).as_posix()}'
    o = f'{pathlib.Path(out_file).as_posix()}'

    sim_argv = f'{i} {o} {add_params}'
    xml_file = to_xml(sim_argv, u)

    sim_exe = os.path.join(clide_root, 'bin', 'cmdline_simulator.exe')
    cmd = f'"{sim_exe}" -project "{xml_file}" -max_cycles 1000000'
    simple_log(cmd)
    execute_cmd(cmd)

    #os.remove(xml_file)
    

def run_test(case):
    in_fname, _ = os.path.splitext(os.path.basename(case[0]))

    test_name = in_fname
    for param in case[1]:
        test_name = f'{test_name}_{param}'

    test_dir = os.path.join(test_outputs, test_name)
    os.makedirs(test_dir, exist_ok=True)

    out_files = []
    for m in models:
        
        out_file = os.path.join(test_dir, f'out_{test_name}_{m}.wav')
        out_files.append(out_file)
        
        if re.findall('(m.*3)', m):
            uld = os.path.join(executables_root, f'{target}_{m}.uld')
            model3_magic(uld, case[0], out_file, case[1])
        else:
            exe = os.path.join(executables_root, f'{target}_{m}.exe')
            execute_test(exe, case[0], out_file, case[1])
    
    with open(os.path.join(test_dir, 'compare_log'), 'w') as log_file:
        for i in range(len(out_files) - 1):
            run_compare(out_files[i], out_files[i+1], log_file)



def run_tests(cases):
    for case in cases:
        run_test(case)


if __name__ == "__main__":

    shutil.rmtree(test_outputs, ignore_errors=True)

    cases = get_cases()

    # print(cases)

    run_tests(cases)


# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2018, Linaro Limited
# Author: Takahiro Akashi <takahiro.akashi@linaro.org>

import os
import os.path
import pytest
import re
from subprocess import call, check_call, check_output, CalledProcessError
from fstest_defs import *

supported_fs_basic = ['fat16', 'fat32', 'ext4']
supported_fs_ext = ['fat16', 'fat32']
supported_fs_mkdir = ['fat16', 'fat32']
supported_fs_unlink = ['fat16', 'fat32']

#
# Filesystem test specific setup
#
def pytest_addoption(parser):
    parser.addoption('--fs-type', action='append', default=None,
        help='Targeting Filesystem Types')

def pytest_configure(config):
    global supported_fs_basic
    global supported_fs_ext
    global supported_fs_mkdir
    global supported_fs_unlink

    def intersect(listA, listB):
        return  [x for x in listA if x in listB]

    supported_fs = config.getoption('fs_type')
    if supported_fs:
        print("*** FS TYPE modified: %s" % supported_fs)
        supported_fs_basic =  intersect(supported_fs, supported_fs_basic)
        supported_fs_ext =  intersect(supported_fs, supported_fs_ext)
        supported_fs_mkdir =  intersect(supported_fs, supported_fs_mkdir)
        supported_fs_unlink =  intersect(supported_fs, supported_fs_unlink)

def pytest_generate_tests(metafunc):
    if 'fs_obj_basic' in metafunc.fixturenames:
        metafunc.parametrize('fs_obj_basic', supported_fs_basic,
            indirect=True, scope='module')
    if 'fs_obj_ext' in metafunc.fixturenames:
        metafunc.parametrize('fs_obj_ext', supported_fs_ext,
            indirect=True, scope='module')
    if 'fs_obj_mkdir' in metafunc.fixturenames:
        metafunc.parametrize('fs_obj_mkdir', supported_fs_mkdir,
            indirect=True, scope='module')
    if 'fs_obj_unlink' in metafunc.fixturenames:
        metafunc.parametrize('fs_obj_unlink', supported_fs_unlink,
            indirect=True, scope='module')

#
# Helper functions
#
def fstype_to_ubname(fs_type):
    if re.match('fat', fs_type):
        return 'fat'
    else:
        return fs_type

def check_ubconfig(config, fs_type):
    if not config.buildconfig.get('config_cmd_%s' % fs_type, None):
        pytest.skip('.config feature "CMD_%s" not enabled' % fs_type.upper())
    if not config.buildconfig.get('config_%s_write' % fs_type, None):
        pytest.skip('.config feature "%s_WRITE" not enabled'
        % fs_type.upper())

def mk_fs(config, fs_type, size, id):
    fs_img = '%s.%s.img' % (id, fs_type)
    fs_img = config.persistent_data_dir + '/' + fs_img

    if fs_type == 'fat16':
        mkfs_opt = '-F 16'
    elif fs_type == 'fat32':
        mkfs_opt = '-F 32'
    else:
        mkfs_opt = ''

    if re.match('fat', fs_type):
        fs_lnxtype = 'vfat'
    else:
        fs_lnxtype = fs_type

    count = (size + 1048576 - 1) / 1048576

    try:
        check_call('rm -f %s' % fs_img, shell=True)
        check_call('dd if=/dev/zero of=%s bs=1M count=%d'
            % (fs_img, count), shell=True)
        check_call('mkfs.%s %s %s'
            % (fs_lnxtype, mkfs_opt, fs_img), shell=True)
        return fs_img
    except CalledProcessError:
        call('rm -f %s' % fs_img, shell=True)
        raise

# from test/py/conftest.py
def tool_is_in_path(tool):
    for path in os.environ["PATH"].split(os.pathsep):
        fn = os.path.join(path, tool)
        if os.path.isfile(fn) and os.access(fn, os.X_OK):
            return True
    return False

fuse_mounted = False

def mount_fs(fs_type, device, mount_point):
    global fuse_mounted

    fuse_mounted = False
    try:
        if tool_is_in_path('guestmount'):
            fuse_mounted = True
            check_call('guestmount -a %s -m /dev/sda %s'
                % (device, mount_point), shell=True)
        else:
            mount_opt = "loop,rw"
            if re.match('fat', fs_type):
                mount_opt += ",umask=0000"

            check_call('sudo mount -o %s %s %s'
                % (mount_opt, device, mount_point), shell=True)

            # may not be effective for some file systems
            check_call('sudo chmod a+rw %s' % mount_point, shell=True)
    except CalledProcessError:
        raise

def umount_fs(fs_type, mount_point):
    if fuse_mounted:
        call('sync')
        call('guestunmount %s' % mount_point, shell=True)
    else:
        call('sudo umount %s' % mount_point, shell=True)

#
# Fixture for basic fs test
#     derived from test/fs/fs-test.sh
#
# NOTE: yield_fixture was deprecated since pytest-3.0
@pytest.yield_fixture()
def fs_obj_basic(request, u_boot_config):
    fs_type = request.param
    fs_img = ''

    fs_ubtype = fstype_to_ubname(fs_type)
    check_ubconfig(u_boot_config, fs_ubtype)

    mount_dir = u_boot_config.persistent_data_dir + '/mnt'

    small_file = mount_dir + '/' + SMALL_FILE
    big_file = mount_dir + '/' + BIG_FILE

    try:

        # 3GiB volume
        fs_img = mk_fs(u_boot_config, fs_type, 0xc0000000, '3GB')

        # Mount the image so we can populate it.
        check_call('mkdir -p %s' % mount_dir, shell=True)
        mount_fs(fs_type, fs_img, mount_dir)

        # Create a subdirectory.
        check_call('mkdir %s/SUBDIR' % mount_dir, shell=True)

        # Create big file in this image.
        # Note that we work only on the start 1MB, couple MBs in the 2GB range
        # and the last 1 MB of the huge 2.5GB file.
        # So, just put random values only in those areas.
        check_call('dd if=/dev/urandom of=%s bs=1M count=1'
	    % big_file, shell=True)
        check_call('dd if=/dev/urandom of=%s bs=1M count=2 seek=2047'
            % big_file, shell=True)
        check_call('dd if=/dev/urandom of=%s bs=1M count=1 seek=2499'
            % big_file, shell=True)

        # Create a small file in this image.
        check_call('dd if=/dev/urandom of=%s bs=1M count=1'
	    % small_file, shell=True)

        # Delete the small file copies which possibly are written as part of a
        # previous test.
        # check_call('rm -f "%s.w"' % MB1, shell=True)
        # check_call('rm -f "%s.w2"' % MB1, shell=True)

        # Generate the md5sums of reads that we will test against small file
        out = check_output(
            'dd if=%s bs=1M skip=0 count=1 2> /dev/null | md5sum'
	    % small_file, shell=True)
        md5val = [ out.split()[0] ]

        # Generate the md5sums of reads that we will test against big file
        # One from beginning of file.
        out = check_output(
            'dd if=%s bs=1M skip=0 count=1 2> /dev/null | md5sum'
	    % big_file, shell=True)
        md5val.append(out.split()[0])

        # One from end of file.
        out = check_output(
            'dd if=%s bs=1M skip=2499 count=1 2> /dev/null | md5sum'
	    % big_file, shell=True)
        md5val.append(out.split()[0])

        # One from the last 1MB chunk of 2GB
        out = check_output(
            'dd if=%s bs=1M skip=2047 count=1 2> /dev/null | md5sum'
	    % big_file, shell=True)
        md5val.append(out.split()[0])

        # One from the start 1MB chunk from 2GB
        out = check_output(
            'dd if=%s bs=1M skip=2048 count=1 2> /dev/null | md5sum'
	    % big_file, shell=True)
        md5val.append(out.split()[0])

        # One 1MB chunk crossing the 2GB boundary
        out = check_output(
            'dd if=%s bs=512K skip=4095 count=2 2> /dev/null | md5sum'
	    % big_file, shell=True)
        md5val.append(out.split()[0])

        umount_fs(fs_type, mount_dir)
    except CalledProcessError:
        pytest.skip('Setup failed for filesystem: ' + fs_type)
        return
    else:
        yield [fs_ubtype, fs_img, md5val]
    finally:
        umount_fs(fs_type, mount_dir)
        call('rmdir %s' % mount_dir, shell=True)
        if fs_img:
            call('rm -f %s' % fs_img, shell=True)

#
# Fixture for extended fs test
#
# NOTE: yield_fixture was deprecated since pytest-3.0
@pytest.yield_fixture()
def fs_obj_ext(request, u_boot_config):
    fs_type = request.param
    fs_img = ''

    fs_ubtype = fstype_to_ubname(fs_type)
    check_ubconfig(u_boot_config, fs_ubtype)

    mount_dir = u_boot_config.persistent_data_dir + '/mnt'

    min_file = mount_dir + '/' + MIN_FILE
    tmp_file = mount_dir + '/tmpfile'

    try:

        # 128MiB volume
        fs_img = mk_fs(u_boot_config, fs_type, 0x8000000, '128MB')

        # Mount the image so we can populate it.
        check_call('mkdir -p %s' % mount_dir, shell=True)
        mount_fs(fs_type, fs_img, mount_dir)

        # Create a test directory
        check_call('mkdir %s/dir1' % mount_dir, shell=True)

        # Create a small file and calculate md5
        check_call('dd if=/dev/urandom of=%s bs=1K count=20'
            % min_file, shell=True)
        out = check_output(
            'dd if=%s bs=1K 2> /dev/null | md5sum'
            % min_file, shell=True)
        md5val = [ out.split()[0] ]

        # Calculate md5sum of Test Case 4
        check_call('dd if=%s of=%s bs=1K count=20'
            % (min_file, tmp_file), shell=True)
        check_call('dd if=%s of=%s bs=1K seek=5 count=20'
            % (min_file, tmp_file), shell=True)
        out = check_output('dd if=%s bs=1K 2> /dev/null | md5sum'
            % tmp_file, shell=True)
        md5val.append(out.split()[0])

        # Calculate md5sum of Test Case 5
        check_call('dd if=%s of=%s bs=1K count=20'
            % (min_file, tmp_file), shell=True)
        check_call('dd if=%s of=%s bs=1K seek=5 count=5'
            % (min_file, tmp_file), shell=True)
        out = check_output('dd if=%s bs=1K 2> /dev/null | md5sum'
            % tmp_file, shell=True)
        md5val.append(out.split()[0])

        # Calculate md5sum of Test Case 7
        check_call('dd if=%s of=%s bs=1K count=20'
            % (min_file, tmp_file), shell=True)
        check_call('dd if=%s of=%s bs=1K seek=20 count=20'
            % (min_file, tmp_file), shell=True)
        out = check_output('dd if=%s bs=1K 2> /dev/null | md5sum'
            % tmp_file, shell=True)
        md5val.append(out.split()[0])

        check_call('rm %s' % tmp_file, shell=True)
        umount_fs(fs_type, mount_dir)
    except CalledProcessError:
        pytest.skip('Setup failed for filesystem: ' + fs_type)
        return
    else:
        yield [fs_ubtype, fs_img, md5val]
    finally:
        umount_fs(fs_type, mount_dir)
        call('rmdir %s' % mount_dir, shell=True)
        if fs_img:
            call('rm -f %s' % fs_img, shell=True)

#
# Fixture for mkdir test
#
# NOTE: yield_fixture was deprecated since pytest-3.0
@pytest.yield_fixture()
def fs_obj_mkdir(request, u_boot_config):
    fs_type = request.param
    fs_img = ''

    fs_ubtype = fstype_to_ubname(fs_type)
    check_ubconfig(u_boot_config, fs_ubtype)

    try:
        # 128MiB volume
        fs_img = mk_fs(u_boot_config, fs_type, 0x8000000, '128MB')
    except:
        pytest.skip('Setup failed for filesystem: ' + fs_type)
    else:
        yield [fs_ubtype, fs_img]
    finally:
        if fs_img:
            call('rm -f %s' % fs_img, shell=True)

#
# Fixture for unlink test
#
# NOTE: yield_fixture was deprecated since pytest-3.0
@pytest.yield_fixture()
def fs_obj_unlink(request, u_boot_config):
    fs_type = request.param
    fs_img = ''

    fs_ubtype = fstype_to_ubname(fs_type)
    check_ubconfig(u_boot_config, fs_ubtype)

    mount_dir = u_boot_config.persistent_data_dir + '/mnt'

    try:

        # 128MiB volume
        fs_img = mk_fs(u_boot_config, fs_type, 0x8000000, '128MB')

        # Mount the image so we can populate it.
        check_call('mkdir -p %s' % mount_dir, shell=True)
        mount_fs(fs_type, fs_img, mount_dir)

        # Test Case 1 & 3
        check_call('mkdir %s/dir1' % mount_dir, shell=True)
        check_call('dd if=/dev/urandom of=%s/dir1/file1 bs=1K count=1'
                                    % mount_dir, shell=True)
        check_call('dd if=/dev/urandom of=%s/dir1/file2 bs=1K count=1'
                                    % mount_dir, shell=True)

        # Test Case 2
        check_call('mkdir %s/dir2' % mount_dir, shell=True)
	for i in range(0, 20):
	    check_call('mkdir %s/dir2/0123456789abcdef%02x'
                                    % (mount_dir, i), shell=True)

        # Test Case 4
        check_call('mkdir %s/dir4' % mount_dir, shell=True)

        # Test Case 5, 6 & 7
        check_call('mkdir %s/dir5' % mount_dir, shell=True)
        check_call('dd if=/dev/urandom of=%s/dir5/file1 bs=1K count=1'
                                    % mount_dir, shell=True)

        umount_fs(fs_type, mount_dir)
    except CalledProcessError:
        pytest.skip('Setup failed for filesystem: ' + fs_type)
        return
    else:
        yield [fs_ubtype, fs_img]
    finally:
        umount_fs(fs_type, mount_dir)
        call('rmdir %s' % mount_dir, shell=True)
        if fs_img:
            call('rm -f %s' % fs_img, shell=True)

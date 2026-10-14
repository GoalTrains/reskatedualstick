"""Export pose diagnostic metadata. Snapshot bytes are unverified candidates.

Usage: python tools/read_pose_diagnostics.py session_controls_pose.bin > pose.csv
The game file is replaced at the next launch. Preserve it before relaunching.
"""
import csv
import struct
import sys

PAD = struct.Struct('<HBBhhhh')

def records(path):
    with open(path, 'rb') as stream:
        header = stream.read(16)
        if len(header) != 16:
            raise ValueError('Incomplete pose header')
        magic, size, version = struct.unpack('<8sII', header)
        if (magic, size, version) not in [(b'SCPOSE01',16536,5101), (b'SCPOSE02',22696,5102), (b'SCPOSE03',28872,5104), (b'SCPOSE04',36104,5107), (b'SCPOSE05',44336,5108)]:
            raise ValueError('Unsupported pose diagnostic format')
        while data := stream.read(size):
            if len(data) != size:
                raise ValueError('Incomplete last snapshot; writer may still be running')
            time, selector, state, label, dropped, valid = struct.unpack_from('<QQIIII', data)
            events, user, enabled, switched, *probes = struct.unpack_from('<4I4i', data, 16480)
            raw = PAD.unpack_from(data, 16512)
            out = PAD.unpack_from(data, 16524)
            context_valid, requests, context_state = 0, '', ''
            upstream_valid = model_valid = property_valid = 0
            properties = [''] * 4
            if version >= 5102:
                context_valid = struct.unpack_from('<I', data, 16536)[0]
                if context_valid & 4:
                    requests = struct.unpack_from('<I',data,16552+0x13d4)[0]
                    context_state = struct.unpack_from('<I',data,16552+0x1414)[0]
            if version >= 5104:
                upstream_valid = struct.unpack_from('<I',data,22696)[0]
            if version >= 5107:
                model_valid = struct.unpack_from('<I',data,28872)[0]
            if version >= 5108:
                property_valid = struct.unpack_from('<I',data,36104)[0]
                for i in range(4):
                    if property_valid & (1 << i):
                        key = struct.unpack_from('<I',data,35080+i*256+0x28)[0]
                        index = (key >> 12) & 0x7ff
                        page = 36144+i*2048
                        occupied = struct.unpack_from('<I',data,page+0x71c+(index>>5)*4)[0]
                        # A missing occupancy bit delegates to parent/default;
                        # it is not a live false value.
                        if occupied & (1 << (index & 31)):
                            properties[i] = data[page+index]
            yield [time, state, events, user, enabled, switched, dropped, valid,
                   *probes, *raw, *out, context_valid, requests, context_state, upstream_valid, model_valid, property_valid, *properties]

if __name__ == '__main__':
    writer = csv.writer(sys.stdout, lineterminator='\n')
    writer.writerow(['qpc_us', 'state', 'event_bits', 'user', 'enabled',
                     'switched', 'dropped', 'valid_block_bits',
                     'probe_a', 'probe_b', 'probe_c', 'probe_d',
                     *['raw_' + x for x in ['buttons','lt','rt','lx','ly','rx','ry']],
                     *['out_' + x for x in ['buttons','lt','rt','lx','ly','rx','ry']],
                     'context_valid_bits','mount_request_bits','context_physics_state','upstream_valid_bits','model_valid_bits',
                     'property_valid_bits','board_backwards','anim_mirrored','brake_mongo','skater_goofy'])
    writer.writerows(records(sys.argv[1]))

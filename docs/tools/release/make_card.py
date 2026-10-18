#!/usr/bin/env python3
"""Builds a complete PiTrex SD card folder from two downloads plus this package.

    python3 make_card.py PiTrex_sd_Jul29_25.zip PitreX_Baremetal_RpiTeaM_Edition.img OUTPUT_FOLDER
                         [--settings tuned] [--arcade-tweaks] [--no-zip]

  * the card contents come from Malban's 2025 zip (the newer baremetal: loader, emulators, programs),
  * the arcade ROM zips (roms/ and roms/aae/) are read out of the RpiTeaM disk image,
  * then install.sh / install.ps1 from this package lays the new menu over it.

Nothing is mounted and no administrator rights are needed: the image is read directly (MBR + FAT32, standard
library only). Needs Python 3.6 or newer. OUTPUT_FOLDER must not exist yet; unless --no-zip is given,
OUTPUT_FOLDER.zip is written next to it. Copy the folder's contents to a FAT32 SD card.
"""
import os, shutil, struct, subprocess, sys, zipfile

HERE = os.path.dirname(os.path.abspath(__file__))


class Fat32:
    """Reads files and folders from a FAT32 partition inside a disk image (read only)."""

    def __init__(self, path):
        self.f = open(path, 'rb')
        self.base = self._find_partition()
        b = self._read(0, 512)
        self.bps, self.spc, self.reserved, self.nfats = struct.unpack_from('<HBHB', b, 11)
        self.spf = struct.unpack_from('<I', b, 36)[0]
        self.root = struct.unpack_from('<I', b, 44)[0]
        if self.bps == 0 or self.spc == 0 or self.spf == 0 or b[510:512] != b'\x55\xaa':
            raise SystemExit('%s: no FAT32 file system found' % path)
        self.fat_start = self.reserved * self.bps
        self.data_start = self.fat_start + self.nfats * self.spf * self.bps
        self.csize = self.spc * self.bps

    def _read(self, off, n):
        self.f.seek(self.base + off)
        return self.f.read(n)

    def _find_partition(self):
        self.f.seek(0)
        mbr = self.f.read(512)
        if mbr[510:512] == b'\x55\xaa':
            for i in range(4):
                e = mbr[446 + 16 * i: 462 + 16 * i]
                if e[4] in (0x0B, 0x0C):
                    return struct.unpack_from('<I', e, 8)[0] * 512
        return 0                                   # no partition table: a bare FAT volume

    def _next(self, c):
        return struct.unpack('<I', self._read(self.fat_start + 4 * c, 4))[0] & 0x0FFFFFFF

    def _chain(self, c):
        while 2 <= c < 0x0FFFFFF8:
            yield c
            c = self._next(c)

    def _cluster(self, c):
        return self._read(self.data_start + (c - 2) * self.csize, self.csize)

    def _entries(self, cluster):
        """Yield (name, is_dir, first_cluster, size) for the entries of a folder."""
        lfn = []
        for c in self._chain(cluster):
            data = self._cluster(c)
            for o in range(0, len(data), 32):
                e = data[o:o + 32]
                if e[0] == 0:
                    return
                if e[0] == 0xE5:
                    lfn = []
                    continue
                if e[11] == 0x0F:
                    part = e[1:11] + e[14:26] + e[28:32]
                    lfn.insert(0, part.decode('utf-16-le').split('\x00')[0].split('￿')[0])
                    continue
                if e[11] & 0x08:                   # volume label
                    lfn = []
                    continue
                if lfn:
                    name = ''.join(lfn)
                else:
                    base, ext = e[0:8].decode('cp437').rstrip(), e[8:11].decode('cp437').rstrip()
                    if e[12] & 0x08: base = base.lower()
                    if e[12] & 0x10: ext = ext.lower()
                    name = base + ('.' + ext if ext else '')
                lfn = []
                if name in ('.', '..'):
                    continue
                first = (struct.unpack_from('<H', e, 20)[0] << 16) | struct.unpack_from('<H', e, 26)[0]
                yield name, bool(e[11] & 0x10), first, struct.unpack_from('<I', e, 28)[0]

    def listdir(self, path):
        cluster = self.root
        for part in [p for p in path.split('/') if p]:
            for name, isdir, first, _ in self._entries(cluster):
                if isdir and name.lower() == part.lower():
                    cluster = first
                    break
            else:
                return None
        return list(self._entries(cluster))

    def copy_file(self, first, size, dest):
        left = size
        with open(dest, 'wb') as out:
            for c in self._chain(first):
                chunk = self._cluster(c)[:left]
                out.write(chunk)
                left -= len(chunk)
                if left <= 0:
                    break


def main(argv):
    sys.stdout.reconfigure(line_buffering=True)
    flags = [a for a in argv if a.startswith('--')]
    args = [a for a in argv if not a.startswith('--')]
    settings = None
    if '--settings' in argv:                       # --settings tuned: the value is not a path
        i = argv.index('--settings')
        settings = argv[i + 1] if i + 1 < len(argv) else ''
        args = [a for a in args if a != settings]
        if settings not in ('keep', 'tuned'):
            raise SystemExit('--settings takes keep or tuned')
    if len(args) != 3:
        print(__doc__)
        return 2
    zip_path, img_path, out = args
    for p in (zip_path, img_path):
        if not os.path.isfile(p):
            raise SystemExit('not a file: %s' % p)
    if os.path.exists(out):
        raise SystemExit('%s exists already; choose a new folder' % out)

    print('1/3 unpacking Malban\'s card from %s' % os.path.basename(zip_path))
    tmp = out + '.unzip.tmp'
    with zipfile.ZipFile(zip_path) as z:
        z.extractall(tmp)
    top = [d for d in os.listdir(tmp) if os.path.isdir(os.path.join(tmp, d))]
    root = os.path.join(tmp, top[0]) if len(top) == 1 and not os.path.exists(os.path.join(tmp, 'pitrex.img')) else tmp
    for need in ('pitrex.img', 'loader.ini', os.path.join('piZero1', 'loader.pit')):
        if not os.path.exists(os.path.join(root, need)):
            shutil.rmtree(tmp, ignore_errors=True)
            raise SystemExit('%s does not look like Malban\'s PiTrex_sd_Jul29_25.zip (%s missing)' % (zip_path, need))
    shutil.move(root, out)
    shutil.rmtree(tmp, ignore_errors=True)

    print('2/3 reading the arcade ROMs from %s' % os.path.basename(img_path))
    fs = Fat32(img_path)
    n = 0
    for sub in ('roms', 'roms/aae'):
        entries = fs.listdir(sub)
        if entries is None:
            print('    warning: no %s folder in the image' % sub)
            continue
        os.makedirs(os.path.join(out, sub), exist_ok=True)
        for name, isdir, first, size in entries:
            if not isdir and name.lower().endswith('.zip'):
                fs.copy_file(first, size, os.path.join(out, sub, name.lower()))
                n += 1
    print('    %d ROM zips copied' % n)

    print('3/3 laying the new menu over it')
    opts = []
    if settings == 'tuned': opts += ['--settings', 'tuned']
    if '--arcade-tweaks' in flags: opts += ['--arcade-tweaks']
    if os.name == 'nt':
        cmd = ['powershell', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', os.path.join(HERE, 'install.ps1')]
        cmd += [{'--settings': '-Settings', '--arcade-tweaks': '-ArcadeTweaks'}.get(o, o) for o in opts] + [out]
    else:
        cmd = ['sh', os.path.join(HERE, 'install.sh')] + opts + [out]
    if subprocess.call(cmd) != 0:
        raise SystemExit('the installer failed; %s is left as it is' % out)
    for d in os.listdir(out):                      # a brand new card needs no way back
        if d.startswith('backup-menu-'):
            shutil.rmtree(os.path.join(out, d))

    if '--no-zip' not in flags:
        print('zipping %s.zip' % out)
        shutil.make_archive(out, 'zip', out)
    print('Done. Copy everything in %s to a FAT32 SD card.' % out)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

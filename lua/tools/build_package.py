from pathlib import Path
import json,zipfile,hashlib,struct,shutil
p=Path(__file__).resolve().parent.parent
files=['manifest.json','reskate-plugin.json','README.md','CHANGELOG.md','LICENSE','icon.png']+[str(x.relative_to(p)).replace('\\','/') for x in sorted((p/'plugin').glob('*.lua'))]
manifest=json.loads((p/'manifest.json').read_text())
plugin=json.loads((p/'reskate-plugin.json').read_text())
assert plugin['entry'] in files and 'input.write' in plugin['permissions']
assert struct.unpack('>II',(p/'icon.png').read_bytes()[16:24])==(256,256)
assert all(Path(x).suffix.lower() not in {'.dll','.exe','.so','.dylib','.pyd'} for x in files)
dist=p/'dist';dist.mkdir(exist_ok=True)
folder=p/'package'/'dual-stick';folder.mkdir(parents=True,exist_ok=True)
for name in files:
 target=folder/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p/name,target)
archive=dist/('Dual_Stick_Controls_Lua_'+manifest['version_number']+'_testing.zip')
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
 for name in files:z.write(p/name,name)
with zipfile.ZipFile(archive) as z:
 assert z.testzip() is None
 assert sorted(z.namelist())==sorted(files)
 for name in z.namelist():assert Path(name).suffix.lower() not in {'.dll','.exe','.so','.dylib','.pyd'}
checksum=hashlib.sha256(archive.read_bytes()).hexdigest()
(dist/'SHA256SUMS.txt').write_text(checksum+'  '+archive.name+'\n')
print(str(archive));print(str(len(files))+' files; no native code; PNG 256x256');print('SHA256 '+checksum)

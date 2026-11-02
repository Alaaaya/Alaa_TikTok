from pathlib import Path
import sys,zipfile,struct,zlib
root=Path(sys.argv[1])/'Fixtures';root.mkdir(parents=True,exist_ok=True)
def png(n):
 def chunk(t,data):return struct.pack('>I',len(data))+t+data+struct.pack('>I',zlib.crc32(t+data))
 w=h=48
 raw=b''.join(b'\0'+bytes([70+n*3%180,100,220,255])*w for y in range(h))
 return b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',w,h,8,6,0,0,0))+chunk(b'IDAT',zlib.compress(raw))+chunk(b'IEND',b'')
with zipfile.ZipFile(root/'good.zip','w',zipfile.ZIP_DEFLATED) as z:
 for n in range(1,47):z.writestr(f'Example/img/shime{n}.png',png(n))
 z.writestr('Example/conf/actions.xml','<Mascot><ActionList><Action Name="Walk"><Animation><Pose Image="/shime3.png" Duration="8" ImageAnchor="64,112"/></Animation></Action></ActionList></Mascot>')
 z.writestr('Example/conf/behaviors.xml','<ignored/>')
for name,files in {
 'traversal.zip':[('../escape/shime1.png',png(1))],
 'duplicate.zip':[('a/shime1.png',png(1)),('b/shime1.png',png(2))],
 'badpng.zip':[('img/shime1.png',b'not PNG')],
 'missing.zip':[('img/shime2.png',png(2))],
 'oversize.zip':[('img/shime1.png',b'A'*(4*1024*1024+1))],
}.items():
 with zipfile.ZipFile(root/name,'w',zipfile.ZIP_DEFLATED) as z:
  for path,data in files:z.writestr(path,data)
with zipfile.ZipFile(root/'symlink.zip','w') as z:
 i=zipfile.ZipInfo('img/shime1.png');i.create_system=3;i.external_attr=(0o120777<<16);z.writestr(i,'/tmp/escape')
(root/'folder'/'img').mkdir(parents=True,exist_ok=True)
(root/'folder'/'img'/'shime1.png').write_bytes(png(1))

for name,xml in {
 'xxe.zip':'<!DOCTYPE x [<!ENTITY ext SYSTEM "file:///etc/passwd">]><Mascot>&ext;</Mascot>',
 'entity.zip':'<!DOCTYPE x [<!ENTITY a "abc">]><Mascot>&a;</Mascot>',
 'deepxml.zip':'<x>'*40+'</x>'*40,
 'badxml.zip':'<Mascot><broken></Mascot>',
 'largexml.zip':' '*1048577,
}.items():
 with zipfile.ZipFile(root/name,'w',zipfile.ZIP_DEFLATED) as z:
  z.writestr('img/shime1.png',png(1));z.writestr('conf/actions.xml',xml)
import shutil
shutil.copyfile(Path(__file__).parent/'fixtures'/'Goku.zip',root/'Goku.zip')

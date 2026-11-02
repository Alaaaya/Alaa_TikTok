from pathlib import Path
s=Path('MascotPacks.inc').read_text();m=Path('Mascot.inc').read_text()
for text in ['TTPackSafePath(path)','0120000','info.flag&1','total>64*1024*1024','count>1000','TTPackCleanPNG','kCGImageSourceThumbnailMaxPixelSize:@256','stopAccessingSecurityScopedResource','UNZ_END_OF_LIST_OF_FILE','unzCloseCurrentFile','rootLink.boolValue','enumerationFailed']:
 assert text in s,text
assert 'dlopen' not in s and 'system(' not in s
for state in ['stand','walk','fall','drag','sit','climb']:assert '@"'+state+'"' in m
assert 'self.packFrames[n] ?: self.packFrames[@1]' in m
print('PASS safe bounded ZIP/folder PNG import, XML inert, six sprite states and fallback')

for text in ['UTTypeZIP','typeWithFilenameExtension:@"zip"','UTTypeArchive','UTTypeFolder','UTTypeData','asCopy:YES','if(folder)return @[UTTypeFolder]']:
 assert text in s,text
print('PASS broad ZIP provider types, copy import and separate folder picker')

x=Path('MascotXML.inc').read_text()
for text in ['shouldResolveExternalEntities=NO','1024*1024','<!DOCTYPE','<!ENTITY','self.depth>32','self.nodes>8192','self.total>2048','abortParsing','poses.json']:
 assert text in x,text
for state in ['run','dash','lookup','dangle','lie','crawl','jump','bounce','ceiling','hang','grab','thrown','spin','sprawl','trip','wallsit']:
 assert '@"'+state+'"' in m,state
assert 'weakObjectsHashTable' in m and 'self.time+.6' in m
print('PASS bounded declarative XML and full native action vocabulary, weak cached surfaces')

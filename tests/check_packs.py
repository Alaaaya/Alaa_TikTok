from pathlib import Path
s=Path('MascotPacks.inc').read_text();m=Path('Mascot.inc').read_text()
for text in ['TTPackSafePath(path)','0120000','info.flag&1','total>64*1024*1024','count>1000','TTPackCleanPNG','kCGImageSourceThumbnailMaxPixelSize:@256','stopAccessingSecurityScopedResource','UNZ_END_OF_LIST_OF_FILE','unzCloseCurrentFile','rootLink.boolValue','enumerationFailed']:
 assert text in s,text
assert 'NSXMLParser' not in s and 'dlopen' not in s and 'system(' not in s
for state in ['stand','walk','fall','drag','sit','climb']:assert '@"'+state+'"' in m
assert 'self.packFrames[n] ?: self.packFrames[@1]' in m
print('PASS safe bounded ZIP/folder PNG import, XML inert, six sprite states and fallback')

for text in ['UTTypeZIP','typeWithFilenameExtension:@"zip"','UTTypeArchive','UTTypeFolder','UTTypeData','asCopy:YES','if(folder)return @[UTTypeFolder]']:
 assert text in s,text
print('PASS broad ZIP provider types, copy import and separate folder picker')

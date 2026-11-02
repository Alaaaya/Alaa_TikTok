// Original pack loader. Only bounded PNG pixels are imported. XML is never executed.
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#import <ImageIO/ImageIO.h>
#include "vendor/ZipArchive/minizip/compat/unzip.h"

static NSString *TTPackRoot(void) {
    return [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject stringByAppendingPathComponent:@"Joker/Mascots"];
}
static NSArray<NSString *> *TTPackIDs(void) {
    NSArray *files=[NSFileManager.defaultManager contentsOfDirectoryAtPath:TTPackRoot() error:nil];
    NSMutableArray *ids=[NSMutableArray new];
    for(NSString *name in files)if(![name hasPrefix:@"."] && [NSFileManager.defaultManager fileExistsAtPath:[[TTPackRoot() stringByAppendingPathComponent:name] stringByAppendingPathComponent:@"shime1.png"]])[ids addObject:name];
    return [ids sortedArrayUsingSelector:@selector(localizedStandardCompare:)];
}
static BOOL TTPackSafePath(NSString *name) {
    if(!name.length || name.length>1024 || [name hasPrefix:@"/"] || [name containsString:@"\\"])return NO;
    for(NSString *part in [name componentsSeparatedByString:@"/"])if([part isEqual:@".."] || [part isEqual:@"."])return NO;
    return YES;
}
static NSInteger TTPackFrameNumber(NSString *name) {
    NSString *leaf=name.lastPathComponent.lowercaseString;
    if(![leaf hasPrefix:@"shime"] || ![leaf hasSuffix:@".png"])return 0;
    NSString *digits=[leaf substringWithRange:NSMakeRange(5,leaf.length-9)];
    NSInteger n=digits.integerValue;
    return n>=1 && n<=46 && [digits isEqualToString:[@(n) stringValue]]?n:0;
}
static NSData *TTPackCleanPNG(NSData *data) {
    if(data.length>4*1024*1024 || data.length<8)return nil;
    const unsigned char *bytes=data.bytes;
    if(memcmp(bytes,"\x89PNG\r\n\x1a\n",8))return nil;
    CGImageSourceRef src=CGImageSourceCreateWithData((__bridge CFDataRef)data,NULL);if(!src)return nil;
    NSDictionary *props=CFBridgingRelease(CGImageSourceCopyPropertiesAtIndex(src,0,NULL));
    NSInteger w=[props[(__bridge NSString *)kCGImagePropertyPixelWidth] integerValue],h=[props[(__bridge NSString *)kCGImagePropertyPixelHeight] integerValue];
    if(w<1 || h<1 || w>2048 || h>2048){CFRelease(src);return nil;}
    CGImageRef cg=CGImageSourceCreateThumbnailAtIndex(src,0,(__bridge CFDictionaryRef)@{(__bridge NSString *)kCGImageSourceCreateThumbnailFromImageAlways:@YES,(__bridge NSString *)kCGImageSourceThumbnailMaxPixelSize:@256});
    CFRelease(src);if(!cg)return nil;
    UIImage *im=[UIImage imageWithCGImage:cg];CGImageRelease(cg);return UIImagePNGRepresentation(im);
}
static BOOL TTPackSaveFrame(NSData *data,NSInteger frame,NSString *stage) {
    NSString *path=[stage stringByAppendingPathComponent:[NSString stringWithFormat:@"shime%ld.png",(long)frame]];
    if([NSFileManager.defaultManager fileExistsAtPath:path])return NO; // Ambiguous multiple characters are rejected.
    NSData *clean=TTPackCleanPNG(data);return clean && [clean writeToFile:path options:NSDataWritingAtomic error:nil];
}
static NSString *TTPackImport(NSURL *url,NSError **error) {
    NSFileManager *fm=NSFileManager.defaultManager;
    [fm createDirectoryAtPath:TTPackRoot() withIntermediateDirectories:YES attributes:nil error:nil];
    NSString *stage=[TTPackRoot() stringByAppendingPathComponent:[@".import-" stringByAppendingString:NSUUID.UUID.UUIDString]];
    [fm createDirectoryAtPath:stage withIntermediateDirectories:YES attributes:nil error:nil];
    BOOL access=[url startAccessingSecurityScopedResource],ok=YES;
    NSNumber *directory=nil;[url getResourceValue:&directory forKey:NSURLIsDirectoryKey error:nil];
    NSUInteger count=0;uint64_t total=0;
    NSNumber *rootLink=nil;[url getResourceValue:&rootLink forKey:NSURLIsSymbolicLinkKey error:nil];if(rootLink.boolValue)ok=NO;
    if(directory.boolValue && ok) {
        __block BOOL enumerationFailed=NO;
        NSDirectoryEnumerator *en=[fm enumeratorAtURL:url includingPropertiesForKeys:@[NSURLIsSymbolicLinkKey,NSURLIsRegularFileKey,NSURLFileSizeKey] options:NSDirectoryEnumerationSkipsHiddenFiles errorHandler:^BOOL(NSURL *u,NSError *e){enumerationFailed=YES;return NO;}];
        for(NSURL *item in en) {
            if(++count>1000){ok=NO;break;}
            NSDictionary *v=[item resourceValuesForKeys:@[NSURLIsSymbolicLinkKey,NSURLIsRegularFileKey,NSURLFileSizeKey] error:nil];
            if(!v || [v[NSURLIsSymbolicLinkKey] boolValue]){ok=NO;break;}
            if(![v[NSURLIsRegularFileKey] boolValue])continue;
            uint64_t size=[v[NSURLFileSizeKey] unsignedLongLongValue];total+=size;
            if(size>4*1024*1024 || total>64*1024*1024){ok=NO;break;}
            NSInteger frame=TTPackFrameNumber(item.lastPathComponent);
            if(frame && !TTPackSaveFrame([NSData dataWithContentsOfURL:item options:NSDataReadingMappedIfSafe error:nil],frame,stage)){ok=NO;break;}
        }
        if(enumerationFailed)ok=NO;
    } else {
        NSDictionary *attrs=[fm attributesOfItemAtPath:url.path error:nil];
        if([attrs fileSize]>32*1024*1024 || [[attrs fileType] isEqual:NSFileTypeSymbolicLink])ok=NO;
        unzFile zip=ok?unzOpen64(url.fileSystemRepresentation):NULL;if(!zip)ok=NO;
        int rc=zip?unzGoToFirstFile(zip):UNZ_BADZIPFILE;
        while(ok && rc==UNZ_OK) {
            unz_file_info64 info={0};char name[1025]={0};
            if(++count>1000 || unzGetCurrentFileInfo64(zip,&info,name,sizeof(name),NULL,0,NULL,0)!=UNZ_OK || info.size_filename>=sizeof(name)){ok=NO;break;}
            NSString *path=[[NSString alloc] initWithBytes:name length:info.size_filename encoding:NSUTF8StringEncoding];
            total+=info.uncompressed_size;
            if(!TTPackSafePath(path) || ((info.external_fa>>16)&0170000)==0120000 || (info.flag&1) ||
               info.uncompressed_size>4*1024*1024 || total>64*1024*1024){ok=NO;break;}
            NSInteger frame=TTPackFrameNumber(path);
            if(frame) {
                if(unzOpenCurrentFile(zip)!=UNZ_OK){ok=NO;break;}
                NSMutableData *data=[NSMutableData new];unsigned char buffer[16384];int n=0;
                while((n=unzReadCurrentFile(zip,buffer,sizeof(buffer)))>0) {
                    if(data.length+(NSUInteger)n>4*1024*1024){ok=NO;break;}
                    [data appendBytes:buffer length:(NSUInteger)n];
                }
                if(n<0 || unzCloseCurrentFile(zip)!=UNZ_OK || !ok || !TTPackSaveFrame(data,frame,stage)){ok=NO;break;}
            }
            rc=unzGoToNextFile(zip);
        }
        if(zip)unzClose(zip);
        if(rc!=UNZ_END_OF_LIST_OF_FILE)ok=NO;
    }
    if(access)[url stopAccessingSecurityScopedResource];
    if(![fm fileExistsAtPath:[stage stringByAppendingPathComponent:@"shime1.png"]])ok=NO;
    NSString *pack=nil;
    if(ok) {
        NSString *base=url.lastPathComponent.stringByDeletingPathExtension;
        base=[[base componentsSeparatedByCharactersInSet:[[NSCharacterSet alphanumericCharacterSet] invertedSet]] componentsJoinedByString:@"-"];
        if(!base.length)base=@"Mascot";if(base.length>40)base=[base substringToIndex:40];
        pack=[base stringByAppendingFormat:@"-%@",[NSUUID.UUID.UUIDString substringToIndex:8]];
        ok=[fm moveItemAtPath:stage toPath:[TTPackRoot() stringByAppendingPathComponent:pack] error:error];
    }
    if(!ok){[fm removeItemAtPath:stage error:nil];if(error && !*error)*error=[NSError errorWithDomain:@"JokerPack" code:1 userInfo:@{NSLocalizedDescriptionKey:@"حزمة غير صالحة. اختار شخصية واحدة فيها shime1.png، بدون روابط أو ملفات كبيرة."}];return nil;}
    return pack;
}
static NSDictionary<NSNumber *,UIImage *> *TTPackLoad(NSString *pack) {
    if(!pack.length || ![TTPackIDs() containsObject:pack])return nil;
    NSMutableDictionary *frames=[NSMutableDictionary new];
    for(NSInteger n=1;n<=46;n++){
        UIImage *im=[UIImage imageWithContentsOfFile:[[TTPackRoot() stringByAppendingPathComponent:pack] stringByAppendingPathComponent:[NSString stringWithFormat:@"shime%ld.png",(long)n]]];
        if(im)frames[@(n)]=im;
    }
    return frames[@1]?frames:nil;
}
// Providers may advertise .zip as an archive, dynamic UTI, or generic data.
static NSArray<UTType *> *TTPackPickerTypes(BOOL folder) {
    if(folder)return @[UTTypeFolder];
    NSMutableArray<UTType *> *types=[NSMutableArray arrayWithObjects:UTTypeZIP,UTTypeArchive,UTTypeFolder,UTTypeData,nil];
    UTType *zipByExtension=[UTType typeWithFilenameExtension:@"zip"];
    if(zipByExtension && ![types containsObject:zipByExtension])[types addObject:zipByExtension];
    return types;
}
@interface TTPackPicker:NSObject<UIDocumentPickerDelegate>
@property(nonatomic,weak) UIViewController *presenter;
@property(nonatomic,copy) void (^completion)(NSString *);
@property(nonatomic) BOOL busy;
+ (instancetype)shared;
- (void)choose:(UIViewController *)vc folder:(BOOL)folder completion:(void (^)(NSString *))completion;
@end
@implementation TTPackPicker
+ (instancetype)shared {static TTPackPicker *p;static dispatch_once_t once;dispatch_once(&once,^{p=[self new];});return p;}
- (void)choose:(UIViewController *)vc folder:(BOOL)folder completion:(void (^)(NSString *))completion {
    if(self.busy || vc.presentedViewController)return;
    self.presenter=vc;self.completion=completion;
    UIDocumentPickerViewController *picker=[[UIDocumentPickerViewController alloc] initForOpeningContentTypes:TTPackPickerTypes(folder) asCopy:YES];
    picker.delegate=self;picker.allowsMultipleSelection=NO;[vc presentViewController:picker animated:YES completion:nil];
}
- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    NSURL *url=urls.firstObject;if(!url || self.busy)return;self.busy=YES;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED,0), ^{
        NSError *error=nil;NSString *pack=TTPackImport(url,&error);
        dispatch_async(dispatch_get_main_queue(), ^{
            self.busy=NO;
            if(pack){if(self.completion)self.completion(pack);}
            else {UIAlertController *alert=[UIAlertController alertControllerWithTitle:@"Joker" message:error.localizedDescription preferredStyle:UIAlertControllerStyleAlert];[alert addAction:[UIAlertAction actionWithTitle:@"تمام" style:UIAlertActionStyleDefault handler:nil]];[self.presenter presentViewController:alert animated:YES completion:nil];}
            self.completion=nil;
        });
    });
}
- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller {self.completion=nil;}
@end

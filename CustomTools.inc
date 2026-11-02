// User-supplied tool art only. No downloaded executable content.
#import <PhotosUI/PhotosUI.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#import <ImageIO/ImageIO.h>
static NSString *TTToolPath(NSInteger tag) {
    if(tag!=1221 && tag!=1222)return nil;
    NSString *dir=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject stringByAppendingPathComponent:@"Joker"];
    return [dir stringByAppendingPathComponent:tag==1221?@"download.png":@"eye.png"];
}
static NSMutableDictionary<NSNumber *,id> *TTToolCache(void) {
    static NSMutableDictionary *cache;static dispatch_once_t once;dispatch_once(&once,^{cache=[NSMutableDictionary new];});return cache;
}
static UIImage *TTToolImage(NSInteger tag) {
    if(tag!=1221 && tag!=1222)return nil;
    id cached=TTToolCache()[@(tag)];
    if(!cached){cached=[[UIImage imageWithContentsOfFile:TTToolPath(tag)] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal] ?: NSNull.null;TTToolCache()[@(tag)]=cached;}
    return [cached isKindOfClass:UIImage.class]?cached:nil;
}
static UIImage *TTToolGlyph(NSInteger tag,NSString *symbol) {
    return TTToolImage(tag) ?: [UIImage systemImageNamed:symbol];
}
static void TTToolRefresh(void);

@interface TTToolPicker : NSObject <PHPickerViewControllerDelegate,UIDocumentPickerDelegate>
@property(nonatomic,weak) UIViewController *presenter;
@property(nonatomic) NSInteger target;
@property(nonatomic) BOOL busy;
+ (instancetype)shared;
- (void)choose:(NSInteger)tag presenter:(UIViewController *)vc source:(UIView *)source;
@end
@implementation TTToolPicker
+ (instancetype)shared {static TTToolPicker *s;static dispatch_once_t once;dispatch_once(&once,^{s=[self new];});return s;}
- (void)message:(NSString *)text {
    UIViewController *vc=self.presenter;if(!vc.view.window)return;
    UIAlertController *alert=[UIAlertController alertControllerWithTitle:@"Joker" message:text preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"تمام" style:UIAlertActionStyleDefault handler:nil]];
    [vc presentViewController:alert animated:YES completion:nil];
}
- (void)choose:(NSInteger)tag presenter:(UIViewController *)vc source:(UIView *)source {
    if(self.busy || vc.presentedViewController || (tag!=1221 && tag!=1222))return;
    self.presenter=vc;self.target=tag;
    UIAlertController *menu=[UIAlertController alertControllerWithTitle:tag==1221?@"Joker · أيقونة التنزيل":@"Joker · أيقونة العين" message:nil preferredStyle:UIAlertControllerStyleActionSheet];
    __weak typeof(self) weak=self;
    [menu addAction:[UIAlertAction actionWithTitle:@"اختيار من الصور" style:UIAlertActionStyleDefault handler:^(__unused UIAlertAction *a){
        PHPickerConfiguration *config=[[PHPickerConfiguration alloc] init];
        config.filter=PHPickerFilter.imagesFilter;config.selectionLimit=1;
        PHPickerViewController *picker=[[PHPickerViewController alloc] initWithConfiguration:config];picker.delegate=weak;
        [vc presentViewController:picker animated:YES completion:nil];
    }]];
    [menu addAction:[UIAlertAction actionWithTitle:@"اختيار من الملفات (PNG / JPG)" style:UIAlertActionStyleDefault handler:^(__unused UIAlertAction *a){
        UIDocumentPickerViewController *picker=[[UIDocumentPickerViewController alloc] initForOpeningContentTypes:@[UTTypePNG,UTTypeJPEG] asCopy:YES];
        picker.allowsMultipleSelection=NO;picker.delegate=weak;[vc presentViewController:picker animated:YES completion:nil];
    }]];
    [menu addAction:[UIAlertAction actionWithTitle:@"إعادة الافتراضي" style:UIAlertActionStyleDestructive handler:^(__unused UIAlertAction *a){
        NSError *error=nil;NSString *path=TTToolPath(weak.target);
        if([NSFileManager.defaultManager fileExistsAtPath:path] && ![NSFileManager.defaultManager removeItemAtPath:path error:&error]){
            [weak message:@"تعذر إزالة الصورة"];return;
        }
        [TTToolCache() removeObjectForKey:@(weak.target)];TTToolRefresh();
    }]];
    [menu addAction:[UIAlertAction actionWithTitle:@"إلغاء" style:UIAlertActionStyleCancel handler:nil]];
    menu.popoverPresentationController.sourceView=source ?: vc.view;
    menu.popoverPresentationController.sourceRect=source?source.bounds:CGRectMake(vc.view.bounds.size.width/2,80,1,1);
    [vc presentViewController:menu animated:YES completion:nil];
}
- (void)importURL:(NSURL *)url target:(NSInteger)target {
    BOOL scoped=[url startAccessingSecurityScopedResource];NSError *error=nil;
    NSNumber *size=nil;[url getResourceValue:&size forKey:NSURLFileSizeKey error:&error];
    if(!size || size.unsignedLongLongValue>20*1024*1024 || !url.isFileURL){
        if(scoped)[url stopAccessingSecurityScopedResource];[self complete:nil target:target];return;
    }
    CGImageSourceRef source=CGImageSourceCreateWithURL((__bridge CFURLRef)url,NULL);
    CGImageRef thumb=source?CGImageSourceCreateThumbnailAtIndex(source,0,(__bridge CFDictionaryRef)@{
        (__bridge NSString *)kCGImageSourceCreateThumbnailFromImageAlways:@YES,
        (__bridge NSString *)kCGImageSourceCreateThumbnailWithTransform:@YES,
        (__bridge NSString *)kCGImageSourceThumbnailMaxPixelSize:@256,
        (__bridge NSString *)kCGImageSourceShouldCacheImmediately:@YES}):NULL;
    UIImage *image=thumb?[UIImage imageWithCGImage:thumb]:nil;
    if(thumb)CGImageRelease(thumb);if(source)CFRelease(source);
    if(scoped)[url stopAccessingSecurityScopedResource];
    if(!image){[self complete:nil target:target];return;}
    // Normalize to a bounded PNG, fit inside the existing glass circle.
    UIGraphicsImageRendererFormat *format=[UIGraphicsImageRendererFormat defaultFormat];format.scale=3;format.opaque=NO;
    UIGraphicsImageRenderer *renderer=[[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(28,28) format:format];
    UIImage *icon=[renderer imageWithActions:^(__unused UIGraphicsImageRendererContext *context){
        CGFloat scale=MIN(28/image.size.width,28/image.size.height);
        CGSize size=CGSizeMake(image.size.width*scale,image.size.height*scale);
        [image drawInRect:CGRectMake((28-size.width)/2,(28-size.height)/2,size.width,size.height)];
    }];
    NSData *data=UIImagePNGRepresentation(icon);NSString *path=TTToolPath(target);
    BOOL ok=[NSFileManager.defaultManager createDirectoryAtPath:path.stringByDeletingLastPathComponent withIntermediateDirectories:YES attributes:nil error:&error];
    if(ok)ok=[data writeToFile:path options:NSDataWritingAtomic error:&error];
    [self complete:ok?[icon imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal]:nil target:target];
}
- (void)complete:(UIImage *)image target:(NSInteger)target {
    dispatch_async(dispatch_get_main_queue(), ^{
        self.busy=NO;
        if(!image){[self message:@"تعذر فتح الصورة. اختار صورة أصغر من 20 ميغابايت."];return;}
        TTToolCache()[@(target)]=image;TTToolRefresh();
    });
}
- (void)picker:(PHPickerViewController *)picker didFinishPicking:(NSArray<PHPickerResult *> *)results {
    [picker dismissViewControllerAnimated:YES completion:nil];
    if(!results.count)return;self.busy=YES;NSInteger target=self.target;
    NSItemProvider *provider=results.firstObject.itemProvider;
    [provider loadFileRepresentationForTypeIdentifier:UTTypeImage.identifier completionHandler:^(NSURL *url,NSError *error){
        // Provider URL expires on callback return, finish reading it here.
        if(url)[self importURL:url target:target];else [self complete:nil target:target];
    }];
}
- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    if(!urls.count)return;self.busy=YES;NSInteger target=self.target;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED,0), ^{[self importURL:urls.firstObject target:target];});
}
@end

@interface TTProfileTools : NSObject <UIGestureRecognizerDelegate>
+ (instancetype)shared;
- (void)attachImage:(UIView *)view;
- (void)attachBio:(UIView *)view;
@end
@implementation TTProfileTools
+ (instancetype)shared { static TTProfileTools *s;static dispatch_once_t once;dispatch_once(&once,^{s=[self new];});return s; }
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)g shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)other { return YES; }
- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)g { return TTExtraEnabled([g.name isEqual:@"AlaaCopyBio"]?@"tt.extra.copyBio":@"tt.extra.profileImage"); }
- (void)attachImage:(UIView *)view {
    if(![view isKindOfClass:UIView.class])return;
    for(UIGestureRecognizer *g in view.gestureRecognizers)if([g.name isEqual:@"AlaaSaveProfileImage"])return;
    UILongPressGestureRecognizer *g=[[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(saveImage:)];
    g.name=@"AlaaSaveProfileImage";g.minimumPressDuration=.65;g.cancelsTouchesInView=NO;g.delegate=self;[view addGestureRecognizer:g];
}
- (void)saveImage:(UILongPressGestureRecognizer *)g {
    if(g.state!=UIGestureRecognizerStateBegan || !TTExtraEnabled(@"tt.extra.profileImage"))return;
    id avatar=TTGActionObject(g.view,@"avatar");
    UIImage *image=[avatar isKindOfClass:UIImageView.class]?((UIImageView *)avatar).image:nil;
    if(!image){[[TT122Hub shared] toast:@"الصورة غير متاحة"];return;}
    if(![NSBundle.mainBundle objectForInfoDictionaryKey:@"NSPhotoLibraryAddUsageDescription"] &&
       ![NSBundle.mainBundle objectForInfoDictionaryKey:@"NSPhotoLibraryUsageDescription"]) {
        [[TT122Hub shared] toast:@"النسخة الموقّعة تحتاج إذن إضافة الصور"];return;
    }
    void (^write)(PHAuthorizationStatus)=^(PHAuthorizationStatus status){
        if(status!=PHAuthorizationStatusAuthorized && status!=PHAuthorizationStatusLimited) {
            dispatch_async(dispatch_get_main_queue(), ^{[[TT122Hub shared] toast:@"اسمح بإضافة الصور من الإعدادات"];});return;
        }
        [PHPhotoLibrary.sharedPhotoLibrary performChanges:^{[PHAssetChangeRequest creationRequestForAssetFromImage:image];}
            completionHandler:^(BOOL ok,NSError *error){dispatch_async(dispatch_get_main_queue(), ^{[[TT122Hub shared] toast:ok?@"تم حفظ صورة الحساب":@"تعذر حفظ صورة الحساب"];});}];
    };
    if(@available(iOS 14,*))[PHPhotoLibrary requestAuthorizationForAccessLevel:PHAccessLevelAddOnly handler:write];
    else [PHPhotoLibrary requestAuthorization:write];
}
- (void)attachBio:(UIView *)view {
    if(![view isKindOfClass:UIView.class])return;
    for(UIGestureRecognizer *g in view.gestureRecognizers)if([g.name isEqual:@"AlaaCopyBio"])return;
    UILongPressGestureRecognizer *g=[[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(copyBio:)];
    g.name=@"AlaaCopyBio";g.minimumPressDuration=.65;g.cancelsTouchesInView=NO;g.delegate=self;[view addGestureRecognizer:g];
}
- (void)copyBio:(UILongPressGestureRecognizer *)g {
    if(g.state!=UIGestureRecognizerStateBegan || !TTExtraEnabled(@"tt.extra.copyBio"))return;
    id vc=TTGActionObject(g.view,@"yy_viewController");
    if(![vc isKindOfClass:NSClassFromString(@"TIKTOKProfileHeaderViewController")])return;
    id bio=TTGActionObject(TTGActionObject(vc,@"user"),@"signature");
    if(![bio isKindOfClass:NSString.class] || ![(NSString *)bio length]) {
        [[TT122Hub shared] toast:@"لا يوجد نبذة للنسخ"];return;
    }
    UIPasteboard.generalPasteboard.string=bio;[[TT122Hub shared] toast:@"تم نسخ النبذة"];
}
@end

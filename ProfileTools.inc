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


// Source-backed model getters; UIKit layout callback avoids guessing private
// configWithUser:isLogSent:shouldUpdateUser: argument encodings.
static char kTTProfileBadge;
static void TTProfileBadge(UIView *header) {
    BOOL showFollow=TTExtraEnabled(@"tt.extra.followsYou"),showCount=TTExtraEnabled(@"tt.extra.videoCount");
    UILabel *badge=objc_getAssociatedObject(header,&kTTProfileBadge);
    if(!showFollow && !showCount) { badge.hidden=YES;return; }
    id vc=TTGActionObject(header,@"parentViewController") ?: TTGActionObject(header,@"yy_viewController");
    if(![vc isKindOfClass:NSClassFromString(@"TIKTOKProfileHeaderViewController")]){badge.hidden=YES;return;}
    id user=TTGActionObject(vc,@"user");NSMutableArray<NSString *> *parts=[NSMutableArray array];
    if(showFollow && TT122Number(user,@"followerStatus")==1)[parts addObject:@"يتابعك"];
    if(showCount) {
        id value=TTGActionObject(user,@"visibleVideosCount");NSString *count=nil;
        if([value isKindOfClass:NSNumber.class] && [(NSNumber *)value longLongValue]>=0)count=[value stringValue];
        else if([value isKindOfClass:NSString.class] && [(NSString *)value length]<16 &&
            [(NSString *)value length] && [[value stringByTrimmingCharactersInSet:NSCharacterSet.decimalDigitCharacterSet] length]==0)count=value;
        if(count)[parts addObject:[NSString stringWithFormat:@"%@ فيديو",count]];
    }
    if(!parts.count || header.bounds.size.width<80){badge.hidden=YES;return;}
    if(!badge) {
        badge=[UILabel new];badge.font=[UIFont systemFontOfSize:11 weight:UIFontWeightSemibold];
        badge.textColor=[UIColor colorWithRed:1 green:.9 blue:.67 alpha:1];
        badge.backgroundColor=[UIColor colorWithWhite:.05 alpha:.8];badge.layer.cornerRadius=8;badge.clipsToBounds=YES;
        badge.textAlignment=NSTextAlignmentCenter;badge.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
        badge.userInteractionEnabled=NO;[header addSubview:badge];objc_setAssociatedObject(header,&kTTProfileBadge,badge,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    NSString *text=[parts componentsJoinedByString:@"  ·  "];if(![badge.text isEqual:text])badge.text=text;
    CGSize size=[badge sizeThatFits:CGSizeMake(header.bounds.size.width-32,24)];
    badge.frame=CGRectMake(16,8,MIN(header.bounds.size.width-32,size.width+18),24);badge.hidden=NO;
    if(header.subviews.lastObject!=badge)[header bringSubviewToFront:badge];
}

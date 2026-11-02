// Phase one is isolated from the 1.21 glyph renderer and tab bar.
#import <Photos/Photos.h>
#import <AVFoundation/AVFoundation.h>

static UIViewController *TT122TopVC(void) {
    UIWindow *window = nil;
    for (UIScene *scene in UIApplication.sharedApplication.connectedScenes) {
        if (scene.activationState != UISceneActivationStateForegroundActive || ![scene isKindOfClass:UIWindowScene.class]) continue;
        for (UIWindow *candidate in ((UIWindowScene *)scene).windows) if (candidate.isKeyWindow) { window = candidate; break; }
        if (window) break;
    }
    UIViewController *vc = window.rootViewController;
    for (NSUInteger i=0;vc && i<20;i++) {
        UIViewController *next = vc.presentedViewController ?: ([vc isKindOfClass:UINavigationController.class] ? ((UINavigationController *)vc).visibleViewController : [vc isKindOfClass:UITabBarController.class] ? ((UITabBarController *)vc).selectedViewController : nil);
        if (!next || next == vc) break; vc = next;
    }
    return vc;
}

static BOOL TT122Enabled(NSString *key) {
    id value = [NSUserDefaults.standardUserDefaults objectForKey:key];
    return value ? [value boolValue] : YES;
}
static NSURL *TT122URL(id value, NSUInteger depth) {
    if (!value || depth > 4) return nil;
    if ([value isKindOfClass:NSURL.class]) {
        NSURL *u = value;
        return [u.scheme.lowercaseString isEqual:@"https"] ? u : nil;
    }
    if ([value isKindOfClass:NSString.class]) return TT122URL([NSURL URLWithString:value], depth + 1);
    if ([value isKindOfClass:NSArray.class]) {
        for (id entry in value) { NSURL *u = TT122URL(entry, depth + 1); if (u) return u; }
        return nil;
    }
    for (NSString *key in @[@"originURLList", @"urlList", @"URLList", @"url", @"URL"]) {
        NSURL *u = TT122URL(TTGActionObject(value, key), depth + 1); if (u) return u;
    }
    return nil;
}
static NSString *TT122Text(id model, NSArray *keys) {
    for (NSString *key in keys) {
        id value = TTGActionObject(model, key);
        if ([value isKindOfClass:NSString.class] && [value length]) return value;
    }
    return nil;
}
static double TT122Number(id object, NSString *key) {
    SEL selector = NSSelectorFromString(key);
    if (![object respondsToSelector:selector]) return 0;
    NSMethodSignature *sig = [object methodSignatureForSelector:selector];
    if (sig.numberOfArguments != 2) return 0;
    const char *type = sig.methodReturnType;
    @try {
        if (type[0] == '@') {
            id value = TTGActionObject(object,key); return [value isKindOfClass:NSNumber.class] ? [value doubleValue] : 0;
        }
        NSInvocation *call = [NSInvocation invocationWithMethodSignature:sig]; call.target=object;call.selector=selector;[call invoke];
        if (!strcmp(type,@encode(double))) { double v=0;[call getReturnValue:&v];return v; }
        if (!strcmp(type,@encode(float))) { float v=0;[call getReturnValue:&v];return v; }
        if (!strcmp(type,@encode(long long))) { long long v=0;[call getReturnValue:&v];return v; }
        if (!strcmp(type,@encode(unsigned long long))) { unsigned long long v=0;[call getReturnValue:&v];return v; }
        if (!strcmp(type,@encode(int))) { int v=0;[call getReturnValue:&v];return v; }
        if (!strcmp(type,@encode(unsigned int))) { unsigned int v=0;[call getReturnValue:&v];return v; }
    } @catch (__unused NSException *e) {}
    return 0;
}
static NSURL *TT122VideoURL(id model, NSInteger quality) {
    id video = TTGActionObject(model, @"video");
    if (quality == 0 && ![D() boolForKey:@"tt.extra.playbackAsset"]) {
        NSURL *normal = TT122URL(TTGActionObject(video, @"downloadURL"), 0);
        if (normal) return normal;
    }
    // Pick only variants actually advertised by this video, never invent CDN URLs.
    id variants = TTGActionObject(video, @"bitRate");
    if (![variants isKindOfClass:NSArray.class]) variants = TTGActionObject(video, @"bitrateInfo");
    NSURL *best = nil; double bestScore = -1;
    if ([variants isKindOfClass:NSArray.class]) {
        for (id variant in variants) {
            NSURL *u = TT122URL(TTGActionObject(variant, @"playURL"), 0);
            if (!u) u = TT122URL(TTGActionObject(variant, @"playAddr"), 0);
            double score = TT122Number(variant, @"bitRate");
            double pixels = TT122Number(variant, @"width") * TT122Number(variant, @"height");
            // HD prefers <=1080p where metadata exists; highest has no cap.
            if (quality == 1 && pixels > 1920.0*1080.0) continue;
            if (u && score >= bestScore) { best = u; bestScore = score; }
        }
    }
    NSURL *playback = best ?: TT122URL(TTGActionObject(video, @"playURL"), 0);
    if ([D() boolForKey:@"tt.extra.playbackAsset"]) return playback;
    return playback ?: TT122URL(TTGActionObject(video, @"downloadURL"), 0);
}
@interface TT122Passthrough : UIView
@end
@implementation TT122Passthrough
- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event {
    UIView *hit = [super hitTest:point withEvent:event]; return hit == self ? nil : hit;
}
@end
@interface TT122Hub : NSObject <NSURLSessionDownloadDelegate>
@property(nonatomic, weak) UIViewController *feed;
@property(nonatomic, weak) id trackedModel;
@property(nonatomic, strong) TT122Passthrough *overlay;
@property(nonatomic, strong) UIButton *downloadButton;
@property(nonatomic, strong) UIButton *eyeButton;
@property(nonatomic, strong) UIButton *progressButton;
@property(nonatomic, strong) CAShapeLayer *ring;
@property(nonatomic, strong) UIView *menu;
@property(nonatomic, strong) id menuModel;
@property(nonatomic, strong) NSURLSession *session;
@property(nonatomic, strong) NSURLSessionDownloadTask *task;
@property(nonatomic, strong) AVAssetExportSession *exporter;
@property(nonatomic, strong) NSURL *sharedAudio;
@property(nonatomic, strong) NSTimer *timer;
@property(nonatomic, weak) id hiddenInteraction;
@property(nonatomic) BOOL uiHidden;
@property(nonatomic) BOOL audio;
@property(nonatomic) BOOL audioAsVideo;
@property(nonatomic, strong) NSURL *coverAudio;
@property(nonatomic) BOOL busy;
@property(nonatomic, strong) NSArray<NSURL *> *audioCandidates;
@property(nonatomic) BOOL dragging;
@property(nonatomic) BOOL started;
@property(nonatomic) BOOL refreshing;
+ (instancetype)shared;
- (void)track:(UIViewController *)controller;
- (void)refresh;
- (void)startLifecycle;
- (void)applyHiddenUI;
- (void)recoverFeed;
- (void)finish:(NSString *)message;
@end
static UIView *TT122Material(CGRect frame) {
    UIVisualEffectView *v = [[UIVisualEffectView alloc] initWithEffect:TTGEffect()];
    v.frame = frame; v.layer.cornerRadius = MIN(22, frame.size.height/2); v.clipsToBounds = YES;
    UIView *tint = [[UIView alloc] initWithFrame:v.bounds];
    tint.autoresizingMask = UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
    tint.backgroundColor = [UIColor colorWithWhite:0 alpha:.38]; tint.userInteractionEnabled = NO;
    [v.contentView addSubview:tint];
    v.layer.borderColor = [UIColor colorWithWhite:1 alpha:.3].CGColor; v.layer.borderWidth = .65;
    return v;
}
@implementation TT122Hub
+ (instancetype)shared { static TT122Hub *hub; static dispatch_once_t once; dispatch_once(&once, ^{hub = [self new];}); return hub; }
- (void)toast:(NSString *)text {
    UIWindow *window = self.overlay.window ?: TT122TopVC().view.window;
    if (!window) return;
    UILabel *label = [UILabel new]; label.text = text; label.numberOfLines = 3;
    label.textColor = UIColor.whiteColor; label.font = [UIFont systemFontOfSize:14 weight:UIFontWeightMedium];
    label.textAlignment = NSTextAlignmentCenter; label.backgroundColor = [UIColor colorWithWhite:.08 alpha:.94];
    label.layer.cornerRadius = 15; label.clipsToBounds = YES;
    label.frame = CGRectMake(24, window.safeAreaInsets.top+60, window.bounds.size.width-48, 68);
    [window addSubview:label];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 3*NSEC_PER_SEC), dispatch_get_main_queue(), ^{[label removeFromSuperview];});
}
- (void)restoreUI {
    if (self.uiHidden) {
        id target = self.hiddenInteraction;
        SEL sel = NSSelectorFromString(@"hideAllElements:exceptArray:");
        NSMethodSignature *s = [target methodSignatureForSelector:sel];
        if (s.numberOfArguments == 4) @try { ((void(*)(id,SEL,BOOL,id))objc_msgSend)(target,sel,NO,nil); } @catch (__unused NSException *e) {}
    }
    self.uiHidden = NO; self.hiddenInteraction = nil;
    [self.eyeButton setImage:TTToolGlyph(1222,@"eye") forState:UIControlStateNormal];
}
- (void)track:(UIViewController *)controller {
    if (!controller.isViewLoaded || !controller.view.window || controller.view.hidden) return;
    CGRect r = [controller.view convertRect:controller.view.bounds toView:controller.view.window];
    CGRect visible = CGRectIntersection(r, controller.view.window.bounds);
    if (CGRectIsNull(visible) || visible.size.height < controller.view.window.bounds.size.height*.5) return;
    id currentModel = TTGActionObject(controller,@"model");
    if (self.feed != controller || self.trackedModel != currentModel) {
        BOOL keepHidden=self.uiHidden;[self restoreUI];self.uiHidden=keepHidden;
        [self.menu removeFromSuperview];self.menu=nil;self.menuModel=nil;self.feed=controller;self.trackedModel=currentModel;
    }
    if(self.uiHidden)[self applyHiddenUI];
    [self startLifecycle];
    [self refresh];

}
- (void)startLifecycle {
    if(self.started)return;self.started=YES;
    NSNotificationCenter *center=NSNotificationCenter.defaultCenter;
    for(NSString *name in @[UIApplicationDidBecomeActiveNotification,UIApplicationWillEnterForegroundNotification,UIWindowDidBecomeKeyNotification,UISceneDidActivateNotification]) {
        [center addObserver:self selector:@selector(lifecycle:) name:name object:nil];
    }
    __weak typeof(self) weak=self;
    self.timer=[NSTimer timerWithTimeInterval:.5 repeats:YES block:^(__unused NSTimer *t){
        if(UIApplication.sharedApplication.applicationState!=UIApplicationStateActive)return;
        [weak recoverFeed];[weak refresh];if(weak.uiHidden)[weak applyHiddenUI];
    }];
    [NSRunLoop.mainRunLoop addTimer:self.timer forMode:NSRunLoopCommonModes];
}
- (void)lifecycle:(NSNotification *)note {
    dispatch_async(dispatch_get_main_queue(), ^{[self recoverFeed];[self refresh];if(self.uiHidden)[self applyHiddenUI];});
}
- (void)recoverFeed {
    UIWindow *window=TT122TopVC().view.window;
    if(!window)return;
    UIViewController *best=nil;CGFloat area=0;
    NSMutableArray<UIView *> *queue=[NSMutableArray arrayWithObject:window];
    for(NSUInteger i=0;i<queue.count && i<1600;i++) {
        UIView *view=queue[i];
        if(view==self.overlay || view.hidden || view.alpha<.01)continue;
        CGRect rect=CGRectIntersection([view convertRect:view.bounds toView:window],window.bounds);
        if(CGRectIsEmpty(rect))continue;
        UIResponder *responder=view.nextResponder;
        if([responder isKindOfClass:UIViewController.class]) {
            UIViewController *vc=(id)responder;NSString *name=NSStringFromClass(vc.class);
            BOOL isFeed=([name containsString:@"Feed"] || [name containsString:@"Aweme"]) &&
                TTGActionObject(vc,@"model") && TTGActionObject(vc,@"interactionController");
            if(isFeed && rect.size.height>window.bounds.size.height*.5 && rect.size.width*rect.size.height>area) {
                best=vc;area=rect.size.width*rect.size.height;
            }
        }
        if(queue.count<1600)[queue addObjectsFromArray:view.subviews];
    }
    if(best)[self track:best];
}
- (void)applyHiddenUI {
    if(!self.uiHidden || !TT122Enabled(@"tt122.eye"))return;
    id interaction=TTGActionObject(self.feed,@"interactionController");
    SEL selector=NSSelectorFromString(@"hideAllElements:exceptArray:");
    NSMethodSignature *sig=[interaction methodSignatureForSelector:selector];
    if(sig.numberOfArguments!=4 || strcmp(sig.methodReturnType,@encode(void)))return;
    const char *flag=[sig getArgumentTypeAtIndex:2],*array=[sig getArgumentTypeAtIndex:3];
    if((strcmp(flag,@encode(BOOL)) && strcmp(flag,"c")) || array[0]!='@')return;
    @try {
        ((void(*)(id,SEL,BOOL,id))objc_msgSend)(interaction,selector,YES,nil);
        self.hiddenInteraction=interaction;
        [self.eyeButton setImage:TTToolGlyph(1222,@"eye.slash") forState:UIControlStateNormal];
    } @catch (__unused NSException *error) {}
}
- (UIButton *)floating:(NSString *)symbol tag:(NSInteger)tag {
    UIButton *b = [UIButton buttonWithType:UIButtonTypeCustom]; b.bounds = CGRectMake(0,0,48,48); b.tag = tag;
    UIView *glass = TT122Material(b.bounds); glass.userInteractionEnabled = NO; glass.autoresizingMask = UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
    [b addSubview:glass]; [b setImage:TTToolGlyph(tag,symbol) forState:UIControlStateNormal];
    b.tintColor = [UIColor colorWithRed:1 green:.92 blue:.72 alpha:1];
    b.accessibilityLabel = tag == 1221 ? @"Download menu" : tag == 1222 ? @"Hide or show feed controls" : @"Download progress";
    [b addTarget:self action:@selector(tapped:) forControlEvents:UIControlEventTouchUpInside];
    [b addGestureRecognizer:[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(pan:)]];
    return b;
}
- (NSString *)positionKey:(UIView *)view { return [NSString stringWithFormat:@"tt122.position.%ld",(long)view.tag]; }
- (CGPoint)clamp:(CGPoint)p view:(UIView *)v {
    UIWindow *w = self.overlay.window;
    CGFloat left = w.safeAreaInsets.left+28, right = w.bounds.size.width-w.safeAreaInsets.right-28;
    CGFloat top = w.safeAreaInsets.top+32, bottom = w.bounds.size.height-w.safeAreaInsets.bottom-100;
    return CGPointMake(MAX(left,MIN(right,p.x)),MAX(top,MIN(bottom,p.y)));
}
- (void)place:(UIView *)v y:(CGFloat)y {
    CGSize s = self.overlay.bounds.size;
    NSDictionary *saved = [NSUserDefaults.standardUserDefaults dictionaryForKey:[self positionKey:v]];
    CGPoint p = saved ? CGPointMake([saved[@"x"] doubleValue]*s.width,[saved[@"y"] doubleValue]*s.height) : CGPointMake(40,y);
    v.center = [self clamp:p view:v];
}
- (void)pan:(UIPanGestureRecognizer *)g {
    UIView *v = g.view; CGPoint delta = [g translationInView:self.overlay];
    self.dragging = g.state == UIGestureRecognizerStateBegan || g.state == UIGestureRecognizerStateChanged;
    v.center = [self clamp:CGPointMake(v.center.x+delta.x,v.center.y+delta.y) view:v];
    [g setTranslation:CGPointZero inView:self.overlay];
    if (g.state == UIGestureRecognizerStateEnded || g.state == UIGestureRecognizerStateCancelled) {
        CGSize s = self.overlay.bounds.size;
        if (s.width && s.height) [NSUserDefaults.standardUserDefaults setObject:@{@"x":@(v.center.x/s.width),@"y":@(v.center.y/s.height)} forKey:[self positionKey:v]];
    }
}
- (void)refresh {
    UIWindow *window = nil;
    for(UIScene *scene in UIApplication.sharedApplication.connectedScenes) {
        if(scene.activationState!=UISceneActivationStateForegroundActive || ![scene isKindOfClass:UIWindowScene.class])continue;
        for(UIWindow *candidate in ((UIWindowScene *)scene).windows)if(candidate.isKeyWindow){window=candidate;break;}
        if(window)break;
    }
    window=window ?: self.feed.viewIfLoaded.window;
    if (!window || UIApplication.sharedApplication.applicationState != UIApplicationStateActive) { self.overlay.hidden = YES; return; }
    if (!self.overlay) {
        self.overlay = [[TT122Passthrough alloc] initWithFrame:window.bounds];
        self.overlay.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
        self.overlay.autoresizingMask = UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
        self.downloadButton = [self floating:@"arrow.down" tag:1221];
        self.eyeButton = [self floating:@"eye" tag:1222];
        self.progressButton = [self floating:@"0.circle" tag:1223];
        [self.overlay addSubview:self.downloadButton]; [self.overlay addSubview:self.eyeButton]; [self.overlay addSubview:self.progressButton];
        self.ring = [CAShapeLayer layer]; self.ring.frame = self.progressButton.bounds;
        self.ring.path = [UIBezierPath bezierPathWithArcCenter:CGPointMake(24,24) radius:20 startAngle:-M_PI_2 endAngle:M_PI*1.5 clockwise:YES].CGPath;
        self.ring.fillColor = UIColor.clearColor.CGColor; self.ring.strokeColor = self.downloadButton.tintColor.CGColor; self.ring.lineWidth = 2.5;
        self.ring.strokeEnd = 0; [self.progressButton.layer addSublayer:self.ring];
    }
    BOOL resized = !CGSizeEqualToSize(self.overlay.bounds.size,window.bounds.size);
    if (self.overlay.superview != window) { self.overlay.frame = window.bounds; [window addSubview:self.overlay]; resized = YES; }
    if (resized || CGPointEqualToPoint(self.downloadButton.center,CGPointZero)) {
        self.overlay.frame = window.bounds;
        [self place:self.downloadButton y:window.bounds.size.height*.42];
        [self place:self.eyeButton y:window.bounds.size.height*.42+64];
        [self place:self.progressButton y:window.bounds.size.height*.42+128];
    }
    UIViewController *top = TT122TopVC();
    BOOL feedTop=NO;
    for(UIViewController *vc=self.feed;vc;vc=vc.parentViewController)if(vc==top){feedTop=YES;break;}
    if(self.feed.presentedViewController || top.presentedViewController)feedTop=NO;
    CGRect r = [self.feed.view convertRect:self.feed.view.bounds toView:window];
    BOOL onScreen = self.feed.view.window==window && CGRectGetHeight(CGRectIntersection(r,window.bounds)) > window.bounds.size.height*.5;
    for(UIView *v=self.feed.view;v;v=v.superview)if(v.hidden || v.alpha<.01){onScreen=NO;break;}
    self.overlay.hidden = !onScreen || !feedTop;
    if(!self.overlay.hidden && self.overlay.superview.subviews.lastObject!=self.overlay)[window bringSubviewToFront:self.overlay];
    self.downloadButton.hidden = !TT122Enabled(@"tt122.download");
    self.eyeButton.hidden = !TT122Enabled(@"tt122.eye");
    self.progressButton.hidden = !self.busy || !TT122Enabled(@"tt122.progress");
    if (!TT122Enabled(@"tt122.eye") && self.uiHidden) [self restoreUI];
    [self.downloadButton setImage:TTToolGlyph(1221,@"arrow.down") forState:UIControlStateNormal];
    [self.eyeButton setImage:TTToolGlyph(1222,self.uiHidden?@"eye.slash":@"eye") forState:UIControlStateNormal];
    self.eyeButton.accessibilityValue=self.uiHidden?@"العناصر مخفية":@"العناصر ظاهرة";
    self.downloadButton.imageView.contentMode=UIViewContentModeScaleAspectFit;
    self.eyeButton.imageView.contentMode=UIViewContentModeScaleAspectFit;
    [self.downloadButton bringSubviewToFront:self.downloadButton.imageView];
    [self.eyeButton bringSubviewToFront:self.eyeButton.imageView];
    // Never bring the overlay over a system permission prompt or the settings sheet.
}
- (void)closeMenu { [self.menu removeFromSuperview]; self.menu = nil; self.menuModel = nil; }
- (void)menuRow:(UIStackView *)stack title:(NSString *)title symbol:(NSString *)symbol action:(void(^)(void))action {
    UIButton *b = [UIButton buttonWithType:UIButtonTypeSystem];
    b.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    b.contentHorizontalAlignment = UIControlContentHorizontalAlignmentLeft; b.tintColor = UIColor.whiteColor;
    b.titleLabel.font = [UIFont systemFontOfSize:16 weight:UIFontWeightMedium];
    [b setTitle:[@"  " stringByAppendingString:title] forState:UIControlStateNormal];
    [b setImage:[UIImage systemImageNamed:symbol] forState:UIControlStateNormal];
    [b addAction:[UIAction actionWithHandler:^(__unused UIAction *a){ action(); }] forControlEvents:UIControlEventTouchUpInside];
    [b.heightAnchor constraintEqualToConstant:44].active = YES; [stack addArrangedSubview:b];
}
- (void)showMenu:(BOOL)copy {
    id model = self.menuModel ?: TTGActionObject(self.feed,@"model");
    if ([NSStringFromClass(self.feed.class) isEqualToString:@"TTKStoryDetailContainerViewController"]) model=TTGActionObject(model,@"currentPlayingStory") ?: model;
    [self.menu removeFromSuperview];
    self.menuModel = model;
    UIView *host = [[UIView alloc] initWithFrame:self.overlay.bounds]; host.autoresizingMask = UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
    self.menu = host; [self.overlay addSubview:host];
    UIButton *outside = [UIButton buttonWithType:UIButtonTypeCustom]; outside.frame = host.bounds; outside.autoresizingMask = host.autoresizingMask;
    [outside addTarget:self action:@selector(closeMenu) forControlEvents:UIControlEventTouchUpInside]; [host addSubview:outside];
    UIView *card = TT122Material(CGRectZero); card.translatesAutoresizingMaskIntoConstraints = NO;
    [host addSubview:card];
    UIView *content = [(UIVisualEffectView *)card contentView];
    UIStackView *stack = [UIStackView new]; stack.axis = UILayoutConstraintAxisVertical; stack.spacing = 3;
    stack.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight; stack.translatesAutoresizingMaskIntoConstraints = NO; [content addSubview:stack];
    [NSLayoutConstraint activateConstraints:@[
        [card.centerXAnchor constraintEqualToAnchor:host.centerXAnchor],
        [card.centerYAnchor constraintEqualToAnchor:host.centerYAnchor constant:-35],
        [card.widthAnchor constraintLessThanOrEqualToConstant:330],
        [card.leftAnchor constraintGreaterThanOrEqualToAnchor:host.leftAnchor constant:22],
        [card.rightAnchor constraintLessThanOrEqualToAnchor:host.rightAnchor constant:-22],
        [card.widthAnchor constraintEqualToConstant:MIN(330,self.overlay.bounds.size.width-44)],
        [stack.topAnchor constraintEqualToAnchor:content.topAnchor constant:16],
        [stack.bottomAnchor constraintEqualToAnchor:content.bottomAnchor constant:-16],
        [stack.leftAnchor constraintEqualToAnchor:content.leftAnchor constant:18],
        [stack.rightAnchor constraintEqualToAnchor:content.rightAnchor constant:-18]]];
    UILabel *brand=[UILabel new];brand.text=copy?@"Joker · نسخ":@"Joker · تنزيل";
    brand.font=[UIFont systemFontOfSize:18 weight:UIFontWeightBold];brand.textColor=UIColor.whiteColor;
    brand.textAlignment=NSTextAlignmentLeft;[stack addArrangedSubview:brand];
    __weak typeof(self) weak = self;
    if (copy) {
        NSArray *titles = @[@"نسخ الاسم",@"نسخ الوصف",@"نسخ اسم المستخدم"];
        for (NSInteger i=0;i<3;i++) {
            [self menuRow:stack title:titles[i] symbol:@"doc.on.doc" action:^{
                id author = TTGActionObject(model,@"author");
                NSString *text = i==0 ? TT122Text(author,@[@"nickname"]) : i==1 ? TT122Text(model,@[@"itemDescription",@"desc",@"awemeDescription"]) : TT122Text(author,@[@"uniqueID",@"uniqueId",@"socialName"]);
                [weak closeMenu];
                if (text.length) { UIPasteboard.generalPasteboard.string = text; [weak toast:@"تم النسخ"]; } else [weak toast:@"النص غير متاح لهذا الفيديو"];
            }];
        }
        [self menuRow:stack title:@"رجوع" symbol:@"chevron.left" action:^{[weak showMenu:NO];}];
    } else {
        NSArray *titles = @[@"تنزيل",@"تنزيل HD",@"أعلى جودة متاحة",@"تنزيل الصوت"];
        NSArray *keys = @[@"tt122.standard",@"tt122.hd",@"tt122.highest",@"tt122.audio"];
        for (NSInteger i=0;i<4;i++) if (TT122Enabled(keys[i])) {
            [self menuRow:stack title:titles[i] symbol:i==3?@"waveform":@"arrow.down" action:^{[weak closeMenu]; [weak start:model quality:i];}];
        }
        if (TT122Enabled(@"tt122.audio")) [self menuRow:stack title:@"حفظ الصوت كفيديو بغلاف" symbol:@"photo.on.rectangle" action:^{[weak closeMenu];[weak start:model quality:4];}];
        if (TT122Enabled(@"tt122.copy")) [self menuRow:stack title:@"نسخ" symbol:@"doc.on.doc" action:^{[weak showMenu:YES];}];
    }
    [self menuRow:stack title:@"إغلاق" symbol:@"xmark" action:^{[weak closeMenu];}];
    host.alpha = 0; card.transform = CGAffineTransformMakeScale(.92,.92);
    [UIView animateWithDuration:.3 delay:0 usingSpringWithDamping:.85 initialSpringVelocity:0 options:0 animations:^{host.alpha=1;card.transform=CGAffineTransformIdentity;} completion:nil];
    // This is a window subview, not a presented controller: playback lifecycle is untouched.
}
- (void)tapped:(UIButton *)sender {
    if (self.dragging) return;
    if (sender == self.downloadButton) { if (self.busy) [self toast:@"التنزيل الحالي قيد التنفيذ"]; else [self showMenu:NO]; }
    else if (sender == self.eyeButton) {
        if (self.uiHidden) { [self restoreUI]; return; }
        self.uiHidden=YES;[self applyHiddenUI];
        if(!self.hiddenInteraction){self.uiHidden=NO;[self toast:@"إخفاء العناصر غير متاح على هذا الفيديو"];}
    } else [self toast:@"التنزيل مستمر"];
}
- (void)start:(id)model quality:(NSInteger)quality {
    if (self.busy) return;
    self.audio = quality >= 3; self.audioAsVideo = quality == 4;
    NSURL *url = TT122VideoURL(model,quality >= 3 ? 0 : quality);
    if (!url) { [self toast:@"رابط الفيديو غير متاح، لا يدعم البث المباشر أو الصور حالياً"]; return; }
    id video = TTGActionObject(model,@"video");
    NSMutableArray *audioURLs = [NSMutableArray array];
    for (NSString *key in @[@"audioURL",@"playURL"]) {
        NSURL *candidate = TT122URL(TTGActionObject(video,key),0);
        if (candidate && ![audioURLs containsObject:candidate]) [audioURLs addObject:candidate];
    }
    self.audioCandidates = audioURLs;
    self.busy = YES; self.ring.strokeEnd = 0;
    [self.progressButton setImage:nil forState:UIControlStateNormal]; [self.progressButton setTitle:@"0%" forState:UIControlStateNormal];
    self.progressButton.titleLabel.font = [UIFont monospacedDigitSystemFontOfSize:11 weight:UIFontWeightSemibold];
    [self refresh];
    NSURLSessionConfiguration *config = NSURLSessionConfiguration.ephemeralSessionConfiguration;
    config.timeoutIntervalForRequest = 45; config.timeoutIntervalForResource = 600;
    self.session = [NSURLSession sessionWithConfiguration:config delegate:self delegateQueue:NSOperationQueue.mainQueue];
    self.task = [self.session downloadTaskWithURL:url]; [self.task resume];
}
- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)task didWriteData:(int64_t)bytes totalBytesWritten:(int64_t)written totalBytesExpectedToWrite:(int64_t)total {
    if (total > 0) {
        CGFloat progress = MIN(.94,(double)written/total*.94);
        [CATransaction begin]; [CATransaction setDisableActions:YES]; self.ring.strokeEnd = progress; [CATransaction commit];
        [self.progressButton setTitle:[NSString stringWithFormat:@"%.0f%%",progress*100] forState:UIControlStateNormal];
    } else [self.progressButton setTitle:@"…" forState:UIControlStateNormal];
}
- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error {
    if (error && self.busy) [self finish:[@"تعذر التنزيل. " stringByAppendingString:error.localizedDescription ?: @""]];
}
- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)task didFinishDownloadingToURL:(NSURL *)location {
    NSInteger status = [(NSHTTPURLResponse *)task.response statusCode];
    if (status < 200 || status >= 300) { [self finish:@"رفض الخادم التنزيل"]; return; }
    NSString *type = task.response.MIMEType.lowercaseString;
    if ([type containsString:@"text"] || [type containsString:@"json"] || [type containsString:@"mpegurl"]) { [self finish:@"هذا الرابط ليس ملف فيديو قابلاً للتنزيل"]; return; }
    NSURL *file = [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:[NSUUID.UUID.UUIDString stringByAppendingString:@".mp4"]]];
    NSError *moveError = nil;
    if (![NSFileManager.defaultManager moveItemAtURL:location toURL:file error:&moveError]) { [self finish:@"تعذر إنشاء ملف التنزيل"]; return; }
    [self.progressButton setTitle:self.audio?@"صوت":@"حفظ" forState:UIControlStateNormal];
    if (self.audio) [self exportAudio:file]; else [self verifyVideo:file];
}
// Never send a silent video-only variant to Photos. Load tracks off the main queue.
- (void)verifyVideo:(NSURL *)file {
    AVURLAsset *asset = [AVURLAsset URLAssetWithURL:file options:nil];
    [asset loadValuesAsynchronouslyForKeys:@[@"tracks",@"duration"] completionHandler:^{
        NSError *error = nil;
        BOOL loaded = [asset statusOfValueForKey:@"tracks" error:&error] == AVKeyValueStatusLoaded;
        NSArray *videoTracks = loaded ? [asset tracksWithMediaType:AVMediaTypeVideo] : @[];
        NSArray *audioTracks = loaded ? [asset tracksWithMediaType:AVMediaTypeAudio] : @[];
        dispatch_async(dispatch_get_main_queue(), ^{
            if (!videoTracks.count) {
                [NSFileManager.defaultManager removeItemAtURL:file error:nil]; [self finish:@"الملف لا يحتوي على فيديو صالح"]; return;
            }
            if (audioTracks.count) { [self normalizeVideo:file asset:asset]; return; }
            [self.progressButton setTitle:@"دمج" forState:UIControlStateNormal];
            [self muxVideo:file asset:asset candidate:0];
        });
    }];
}
- (void)normalizeVideo:(NSURL *)file asset:(AVAsset *)asset {
    // Always output an actual MPEG-4 container, not just a renamed CDN file.
    AVAssetExportSession *exporter=[[AVAssetExportSession alloc] initWithAsset:asset presetName:AVAssetExportPresetPassthrough];
    if (![exporter.supportedFileTypes containsObject:AVFileTypeMPEG4])
        exporter=[[AVAssetExportSession alloc] initWithAsset:asset presetName:AVAssetExportPresetHighestQuality];
    if (!exporter || ![exporter.supportedFileTypes containsObject:AVFileTypeMPEG4]) {
        [NSFileManager.defaultManager removeItemAtURL:file error:nil];[self finish:@"تعذر إنشاء MP4 مع الصوت"];return;
    }
    NSURL *output=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:[NSUUID.UUID.UUIDString stringByAppendingString:@".mp4"]]];
    exporter.outputURL=output;exporter.outputFileType=AVFileTypeMPEG4;self.exporter=exporter;
    [exporter exportAsynchronouslyWithCompletionHandler:^{
        [NSFileManager.defaultManager removeItemAtURL:file error:nil];
        dispatch_async(dispatch_get_main_queue(), ^{
            self.exporter=nil;
            if (exporter.status!=AVAssetExportSessionStatusCompleted) {
                [NSFileManager.defaultManager removeItemAtURL:output error:nil];[self finish:@"تعذر تجهيز فيديو MP4 مع الصوت"];return;
            }
            [self verifyMuxedVideo:output];
        });
    }];
}
- (void)muxVideo:(NSURL *)file asset:(AVURLAsset *)video candidate:(NSUInteger)index {
    if (index >= self.audioCandidates.count) {
        [NSFileManager.defaultManager removeItemAtURL:file error:nil];
        [self finish:@"لم يتوفر مسار صوت مطابق، لم يُحفظ فيديو صامت"]; return;
    }
    NSURL *source = self.audioCandidates[index];
    AVURLAsset *audioAsset = [AVURLAsset URLAssetWithURL:source options:nil];
    [audioAsset loadValuesAsynchronouslyForKeys:@[@"tracks",@"duration"] completionHandler:^{
        NSError *error = nil;
        BOOL loaded = [audioAsset statusOfValueForKey:@"tracks" error:&error] == AVKeyValueStatusLoaded &&
            [audioAsset statusOfValueForKey:@"duration" error:&error] == AVKeyValueStatusLoaded;
        AVAssetTrack *audioTrack = loaded ? [audioAsset tracksWithMediaType:AVMediaTypeAudio].firstObject : nil;
        AVAssetTrack *videoTrack = [video tracksWithMediaType:AVMediaTypeVideo].firstObject;
        CMTime duration = video.duration;
        double seconds = CMTimeGetSeconds(duration);
        BOOL enough = audioTrack && isfinite(seconds) && seconds > 0 &&
            CMTimeGetSeconds(audioAsset.duration) + .1 >= seconds;
        if (!enough) {
            dispatch_async(dispatch_get_main_queue(), ^{[self muxVideo:file asset:video candidate:index+1];}); return;
        }
        AVMutableComposition *composition = [AVMutableComposition composition];
        AVMutableCompositionTrack *v = [composition addMutableTrackWithMediaType:AVMediaTypeVideo preferredTrackID:kCMPersistentTrackID_Invalid];
        AVMutableCompositionTrack *a = [composition addMutableTrackWithMediaType:AVMediaTypeAudio preferredTrackID:kCMPersistentTrackID_Invalid];
        CMTimeRange range = CMTimeRangeMake(kCMTimeZero,duration);
        BOOL inserted = [v insertTimeRange:range ofTrack:videoTrack atTime:kCMTimeZero error:&error] &&
            [a insertTimeRange:range ofTrack:audioTrack atTime:kCMTimeZero error:&error];
        v.preferredTransform = videoTrack.preferredTransform;
        if (!inserted) {
            dispatch_async(dispatch_get_main_queue(), ^{[self muxVideo:file asset:video candidate:index+1];}); return;
        }
        AVAssetExportSession *exporter = [[AVAssetExportSession alloc] initWithAsset:composition presetName:AVAssetExportPresetPassthrough];
        if (![exporter.supportedFileTypes containsObject:AVFileTypeMPEG4])
            exporter = [[AVAssetExportSession alloc] initWithAsset:composition presetName:AVAssetExportPresetHighestQuality];
        if (!exporter || ![exporter.supportedFileTypes containsObject:AVFileTypeMPEG4]) {
            dispatch_async(dispatch_get_main_queue(), ^{[self muxVideo:file asset:video candidate:index+1];}); return;
        }
        NSURL *output = [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:[NSUUID.UUID.UUIDString stringByAppendingString:@".mp4"]]];
        exporter.outputURL=output;exporter.outputFileType=AVFileTypeMPEG4;exporter.shouldOptimizeForNetworkUse=YES;
        AVAssetExportSession *active=exporter;
        dispatch_async(dispatch_get_main_queue(), ^{self.exporter=active;});
        [active exportAsynchronouslyWithCompletionHandler:^{
            dispatch_async(dispatch_get_main_queue(), ^{
                self.exporter=nil;
                if (active.status != AVAssetExportSessionStatusCompleted) {
                    [NSFileManager.defaultManager removeItemAtURL:output error:nil];
                    [self muxVideo:file asset:video candidate:index+1]; return;
                }
                [NSFileManager.defaultManager removeItemAtURL:file error:nil];
                [self verifyMuxedVideo:output];
            });
        }];
    }];
}
- (void)verifyMuxedVideo:(NSURL *)file {
    AVURLAsset *asset = [AVURLAsset URLAssetWithURL:file options:nil];
    [asset loadValuesAsynchronouslyForKeys:@[@"tracks"] completionHandler:^{
        BOOL ok = [asset statusOfValueForKey:@"tracks" error:nil] == AVKeyValueStatusLoaded &&
            [asset tracksWithMediaType:AVMediaTypeAudio].count && [asset tracksWithMediaType:AVMediaTypeVideo].count;
        dispatch_async(dispatch_get_main_queue(), ^{
            if (ok) [self saveVideo:file];
            else { [NSFileManager.defaultManager removeItemAtURL:file error:nil]; [self finish:@"فشل فحص الصوت بعد الدمج، لم يتم الحفظ"]; }
        });
    }];
}
- (void)saveVideo:(NSURL *)file {
    // A tweak cannot safely supply a missing host privacy usage string.
    NSBundle *bundle = NSBundle.mainBundle;
    BOOL addOnly = [[bundle objectForInfoDictionaryKey:@"NSPhotoLibraryAddUsageDescription"] length] > 0;
    BOOL readWrite = [[bundle objectForInfoDictionaryKey:@"NSPhotoLibraryUsageDescription"] length] > 0;
    if (!addOnly && !readWrite) {
        [NSFileManager.defaultManager removeItemAtURL:file error:nil];
        [self finish:@"نسخة التطبيق تفتقد إذن حفظ الصور، يلزم إضافته عند توقيع التطبيق"]; return;
    }
    PHAccessLevel access = addOnly ? PHAccessLevelAddOnly : PHAccessLevelReadWrite;
    [PHPhotoLibrary requestAuthorizationForAccessLevel:access handler:^(PHAuthorizationStatus status) {
        if (status != PHAuthorizationStatusAuthorized && status != PHAuthorizationStatusLimited) {
            [NSFileManager.defaultManager removeItemAtURL:file error:nil];
            dispatch_async(dispatch_get_main_queue(), ^{[self finish:@"اسمح لـ TikTok بحفظ الفيديو من إعدادات الصور"];}); return;
        }
        [PHPhotoLibrary.sharedPhotoLibrary performChanges:^{
            [PHAssetChangeRequest creationRequestForAssetFromVideoAtFileURL:file];
        } completionHandler:^(BOOL ok,NSError *error){
            [NSFileManager.defaultManager removeItemAtURL:file error:nil];
            dispatch_async(dispatch_get_main_queue(), ^{[self finish:ok?@"تم الحفظ في الصور":@"تعذر حفظ هذا الملف في الصور"];});
        }];
    }];
}
- (void)exportAudio:(NSURL *)file {
    AVURLAsset *asset = [AVURLAsset URLAssetWithURL:file options:nil];
    AVAssetExportSession *exporter = [[AVAssetExportSession alloc] initWithAsset:asset presetName:AVAssetExportPresetAppleM4A];
    if (!exporter) { [NSFileManager.defaultManager removeItemAtURL:file error:nil]; [self finish:@"تعذر استخراج الصوت"]; return; }
    NSURL *audio = [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:[NSUUID.UUID.UUIDString stringByAppendingString:@".m4a"]]];
    exporter.outputURL=audio;exporter.outputFileType=AVFileTypeAppleM4A; self.exporter=exporter;
    [exporter exportAsynchronouslyWithCompletionHandler:^{
        UIImage *cover = nil;
        if (self.audioAsVideo && exporter.status == AVAssetExportSessionStatusCompleted) {
            AVAssetImageGenerator *generator = [AVAssetImageGenerator assetImageGeneratorWithAsset:asset];
            generator.appliesPreferredTrackTransform=YES; generator.maximumSize=CGSizeMake(720,720);
            CGImageRef image=[generator copyCGImageAtTime:kCMTimeZero actualTime:NULL error:nil];
            if (image) { cover=[UIImage imageWithCGImage:image];CGImageRelease(image); }
        }
        [NSFileManager.defaultManager removeItemAtURL:file error:nil];
        dispatch_async(dispatch_get_main_queue(), ^{
            BOOL ok = exporter.status == AVAssetExportSessionStatusCompleted; self.exporter=nil;
            if (ok && self.audioAsVideo) { [self renderAudioCover:audio image:cover]; return; }
            [self finish:ok?@"الصوت جاهز للمشاركة":@"تعذر استخراج الصوت من الفيديو"];
            if (!ok) { [NSFileManager.defaultManager removeItemAtURL:audio error:nil]; return; }
            // Native share sheet offers Save to Files and installed receiving apps.
            self.sharedAudio = audio;
            UIActivityViewController *share = [[UIActivityViewController alloc] initWithActivityItems:@[audio] applicationActivities:nil];
            share.popoverPresentationController.sourceView=self.downloadButton;
            share.popoverPresentationController.sourceRect=self.downloadButton.bounds;
            share.completionWithItemsHandler=^(UIActivityType activity,BOOL done,NSArray *items,NSError *error){
                [NSFileManager.defaultManager removeItemAtURL:audio error:nil]; self.sharedAudio=nil;
            };
            UIViewController *presenter=TT122TopVC();
            if (!presenter || presenter.isBeingDismissed || presenter.isBeingPresented) {
                [NSFileManager.defaultManager removeItemAtURL:audio error:nil];self.sharedAudio=nil;[self toast:@"تعذر فتح المشاركة حالياً"];return;
            }
            [presenter presentViewController:share animated:YES completion:nil];
        });
    }];
}
// Generate a low-frame-rate static cover, then passthrough-mux the full M4A.
- (void)renderAudioCover:(NSURL *)audio image:(UIImage *)image {
    self.coverAudio=audio;self.audioCandidates=@[audio];
    [self.progressButton setTitle:@"غلاف" forState:UIControlStateNormal];
    AVURLAsset *asset=[AVURLAsset URLAssetWithURL:audio options:nil];
    [asset loadValuesAsynchronouslyForKeys:@[@"duration"] completionHandler:^{
        CMTime duration=asset.duration;double seconds=CMTimeGetSeconds(duration);
        if ([asset statusOfValueForKey:@"duration" error:nil]!=AVKeyValueStatusLoaded || !isfinite(seconds) || seconds<=0) {
            dispatch_async(dispatch_get_main_queue(), ^{[self finish:@"تعذر قراءة مدة الصوت"];});return;
        }
        NSURL *output=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:[NSUUID.UUID.UUIDString stringByAppendingString:@".mp4"]]];
        NSError *error=nil;
        AVAssetWriter *writer=[[AVAssetWriter alloc] initWithURL:output fileType:AVFileTypeMPEG4 error:&error];
        AVAssetWriterInput *input=[AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo outputSettings:@{AVVideoCodecKey:AVVideoCodecTypeH264,AVVideoWidthKey:@720,AVVideoHeightKey:@720}];
        input.expectsMediaDataInRealTime=NO;
        AVAssetWriterInputPixelBufferAdaptor *adaptor=[AVAssetWriterInputPixelBufferAdaptor assetWriterInputPixelBufferAdaptorWithAssetWriterInput:input sourcePixelBufferAttributes:@{(NSString *)kCVPixelBufferPixelFormatTypeKey:@(kCVPixelFormatType_32ARGB),(NSString *)kCVPixelBufferWidthKey:@720,(NSString *)kCVPixelBufferHeightKey:@720,(NSString *)kCVPixelBufferCGImageCompatibilityKey:@YES,(NSString *)kCVPixelBufferCGBitmapContextCompatibilityKey:@YES}];
        if (!writer || ![writer canAddInput:input]) {
            dispatch_async(dispatch_get_main_queue(), ^{[self finish:@"تعذر إنشاء فيديو الغلاف"];});return;
        }
        [writer addInput:input];
        if (![writer startWriting]) {
            [NSFileManager.defaultManager removeItemAtURL:output error:nil];
            dispatch_async(dispatch_get_main_queue(), ^{[self finish:@"تعذر بدء فيديو الغلاف"];});return;
        }
        [writer startSessionAtSourceTime:kCMTimeZero];
        CVPixelBufferRef pixel=NULL;
        CVReturn result=CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault,adaptor.pixelBufferPool,&pixel);
        if (result!=kCVReturnSuccess || !pixel) {
            [writer cancelWriting];[NSFileManager.defaultManager removeItemAtURL:output error:nil];
            dispatch_async(dispatch_get_main_queue(), ^{[self finish:@"تعذر رسم الغلاف"];});return;
        }
        CVPixelBufferLockBaseAddress(pixel,0);
        CGColorSpaceRef space=CGColorSpaceCreateDeviceRGB();
        CGContextRef context=CGBitmapContextCreate(CVPixelBufferGetBaseAddress(pixel),720,720,8,CVPixelBufferGetBytesPerRow(pixel),space,kCGImageAlphaNoneSkipFirst);
        CGColorSpaceRelease(space);
        if (!context) {
            CVPixelBufferUnlockBaseAddress(pixel,0);CVPixelBufferRelease(pixel);[writer cancelWriting];
            [NSFileManager.defaultManager removeItemAtURL:output error:nil];
            dispatch_async(dispatch_get_main_queue(), ^{[self finish:@"تعذر رسم الغلاف"];});return;
        }
        CGContextSetRGBFillColor(context,.06,.06,.07,1);CGContextFillRect(context,CGRectMake(0,0,720,720));
        if (image.CGImage) {
            CGFloat w=CGImageGetWidth(image.CGImage),h=CGImageGetHeight(image.CGImage),scale=MIN(720/w,720/h);
            CGContextTranslateCTM(context,0,720);CGContextScaleCTM(context,1,-1);
            CGContextDrawImage(context,CGRectMake((720-w*scale)/2,(720-h*scale)/2,w*scale,h*scale),image.CGImage);
        }
        CGContextRelease(context);CVPixelBufferUnlockBaseAddress(pixel,0);
        __block NSInteger frame=0;__block BOOL ended=NO;
        NSInteger total=(NSInteger)ceil(seconds);
        dispatch_queue_t queue=dispatch_queue_create("com.alaa.tiktok.cover",DISPATCH_QUEUE_SERIAL);
        [input requestMediaDataWhenReadyOnQueue:queue usingBlock:^{
            if (ended) return;
            while (input.readyForMoreMediaData && frame<total) {
                if (![adaptor appendPixelBuffer:pixel withPresentationTime:CMTimeMake(frame,1)]) {
                    ended=YES;CVPixelBufferRelease(pixel);[writer cancelWriting];
                    [NSFileManager.defaultManager removeItemAtURL:output error:nil];
                    dispatch_async(dispatch_get_main_queue(), ^{[self finish:@"تعذر كتابة الغلاف"];});return;
                }
                frame++;
            }
            if (frame>=total && !ended) {
                ended=YES;CVPixelBufferRelease(pixel);[input markAsFinished];[writer endSessionAtSourceTime:duration];
                [writer finishWritingWithCompletionHandler:^{
                    dispatch_async(dispatch_get_main_queue(), ^{
                        if (writer.status!=AVAssetWriterStatusCompleted) {
                            [NSFileManager.defaultManager removeItemAtURL:output error:nil];[self finish:@"تعذر إكمال فيديو الغلاف"];return;
                        }
                        [self verifyVideo:output];
                    });
                }];
            }
        }];
    }];
}
- (void)finish:(NSString *)message {
    if (self.coverAudio) { [NSFileManager.defaultManager removeItemAtURL:self.coverAudio error:nil];self.coverAudio=nil; }
    self.busy=NO; self.task=nil; self.audioCandidates=nil; [self.session finishTasksAndInvalidate]; self.session=nil;
    self.ring.strokeEnd=1; [self refresh]; [self toast:message];
}
@end


static void TTToolRefresh(void) { [[TT122Hub shared] refresh]; }

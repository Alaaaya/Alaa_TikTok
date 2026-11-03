// 58: cosmetic restyle inside native action controls. Native hit regions, targets,
// recognizers, counts and frames remain intact. Not a full HTML overlay.
static NSString *const JKStyleKey=@"tt.interface.style";
static const void *kJKPaintAlpha=&kJKPaintAlpha;
static NSUInteger JKStyled,JKSkipped;
static NSMutableDictionary *JKReasons;
static void JKResult(BOOL styled,NSString *reason){if(styled)JKStyled++;else JKSkipped++;if(JKReasons.count<16 || JKReasons[reason])JKReasons[reason]=@([JKReasons[reason] unsignedIntegerValue]+1);}
static const void *kJKItemKey=&kJKItemKey,*kJKOrigAlphaKey=&kJKOrigAlphaKey;
typedef NS_ENUM(NSInteger,JKKind){JKKindNone=0,JKKindLike,JKKindComment,JKKindSave,JKKindShare};
static NSInteger JKStyle(void){return [[NSUserDefaults standardUserDefaults] integerForKey:JKStyleKey];}
// 0 TikTok, 1 Instagram Reels are shipped. 2 Facebook (53) and 3 Joker futuristic are not available yet.
static BOOL JKStyleAvailable(NSInteger style){return style==0 || style==1 || style==3;}
// Release gate: Instagram rail ships in 53 (picker offers TikTok + Instagram; Facebook 54, Joker style after mockup approval).
#ifndef JK_INSTA_SHIPPED
#define JK_INSTA_SHIPPED 1
#endif
static BOOL JKPickerAvailable(NSInteger style){return style==0 || (JK_INSTA_SHIPPED && style==1) || style==3;}
static BOOL JKStyleOn(void){NSInteger s=JKStyle();return (s==1 || s==3) && JKStyleAvailable(s);}
static BOOL JKAlaa(void){return JKStyle()==3;}
static id JKObject(id object,NSString *name) {
    SEL selector=NSSelectorFromString(name);
    if(!object || ![object respondsToSelector:selector])return nil;
    NSMethodSignature *sig=[object methodSignatureForSelector:selector];
    if(!sig || sig.numberOfArguments!=2 || sig.methodReturnType[0]!='@')return nil;
    @try {return ((id(*)(id,SEL))objc_msgSend)(object,selector);} @catch(__unused NSException *e) {return nil;}
}
static void JKIdentity(UIView *node,NSMutableString *out,NSUInteger depth) {
    if(depth>2 || [node isKindOfClass:UILabel.class] || [node isKindOfClass:UIVisualEffectView.class])return;
    if(node.bounds.size.width<=80 && node.bounds.size.height<=80) {
        [out appendFormat:@" %@",NSStringFromClass(node.class)];
        for(NSString *key in @[@"imageNameString",@"accessibilityIdentifier"]) {
            id v=JKObject(node,key);if([v isKindOfClass:NSString.class])[out appendFormat:@" %@",v];
        }
    }
    for(UIView *child in node.subviews)JKIdentity(child,out,depth+1);
}
static JKKind JKKindOf(UIView *button) {
    if([button isKindOfClass:UILabel.class] || [button isKindOfClass:UIImageView.class])return JKKindNone;
    NSMutableString *identity=[NSMutableString stringWithString:NSStringFromClass(button.class)];
    if([NSStringFromClass(button.superview.class) containsString:@"PlayInteractionLikeView"])[identity appendString:@" like"];
    for(NSString *key in @[@"imageNameString",@"accessibilityIdentifier",@"accessibilityLabel"]) {
        id v=JKObject(button,key);if([v isKindOfClass:NSString.class])[identity appendFormat:@" %@",v];
    }
    for(UIView *child in button.subviews)JKIdentity(child,identity,0);
    NSString *name=[identity stringByFoldingWithOptions:NSDiacriticInsensitiveSearch locale:nil].lowercaseString;
    for(NSString *bad in @[@"avatar",@"follow",@"music",@"disc",@"download",@"folgen",@"musik",@"متابعة",@"موسيقى"])
        if([name containsString:bad])return JKKindNone;
    for(NSString *k in @[@"comment",@"kommentar",@"تعليق"])if([name containsString:k])return JKKindComment;
    for(NSString *k in @[@"share",@"forward",@"teilen",@"مشاركة"])if([name containsString:k])return JKKindShare;
    for(NSString *k in @[@"collect",@"favorite",@"favourite",@"bookmark",@"save",@"favoriten",@"speichern",@"مفضلة",@"حفظ"])
        if([name containsString:k])return JKKindSave;
    for(NSString *k in @[@"like",@"digg",@"heart",@"gefallt mir",@"اعجاب"])if([name containsString:k])return JKKindLike;
    return JKKindNone;
}
// Same feed-column rules as the glass glyphs, minus the alpha test (our own hiding sets alpha 0).
static BOOL JKInFeed(UIView *button) {
    UIWindow *window=button.window;
    if(!window || button.bounds.size.width<24 || button.bounds.size.width>100 ||
       button.bounds.size.height<24 || button.bounds.size.height>150)return NO;
    CGRect r=[button convertRect:button.bounds toView:window];
    if((CGRectGetMidX(r)>window.bounds.size.width*.30 && CGRectGetMidX(r)<window.bounds.size.width*.70) ||
       CGRectGetMidY(r)<80 || CGRectGetMidY(r)>window.bounds.size.height-85)return NO;
    BOOL feed=NO;
    for(UIView *p=button.superview;p;p=p.superview) {
        NSString *n=NSStringFromClass(p.class).lowercaseString;
        if([p isKindOfClass:UITabBar.class] || [n containsString:@"commentpanel"] || [n containsString:@"commentsheet"] ||
           [n containsString:@"profile"] || [n containsString:@"settings"])return NO;
        if([n containsString:@"feed"] || [n containsString:@"playinteraction"])feed=YES;
    }
    return feed;
}
static BOOL JKCanForward(UIView *native) {
    if(![native isKindOfClass:UIControl.class] || !((UIControl *)native).enabled)return NO;
    UIControl *c=(UIControl *)native;
    for(id target in c.allTargets) {
        id t=[target isKindOfClass:NSNull.class]?nil:target;
        if([c actionsForTarget:t forControlEvent:UIControlEventTouchUpInside].count)return YES;
    }
    return NO;
}
static NSString *JKCountText(UIView *view,NSUInteger depth) {
    if(depth>4)return nil;
    if([view isKindOfClass:UILabel.class]) {
        NSString *t=((UILabel *)view).text;
        return t.length && [t rangeOfCharacterFromSet:NSCharacterSet.decimalDigitCharacterSet].location!=NSNotFound?t:nil;
    }
    for(UIView *c in view.subviews){NSString *t=JKCountText(c,depth+1);if(t)return t;}
    return nil;
}

// 56: Alaa style collapsible rail. The toggle only shows/hides OUR overlay glyphs; native views are never touched.
static BOOL JKRailClosed=YES;
@interface JKAlaaToggle:UIControl
@property(nonatomic,strong) UIImageView *dots,*xmark;
@end
@implementation JKAlaaToggle
- (instancetype)initWithFrame:(CGRect)f {
    self=[super initWithFrame:f];if(!self)return nil;
    self.tag=9156;self.accessibilityLabel=@"Rail";self.accessibilityTraits=UIAccessibilityTraitButton;
    self.layer.cornerRadius=24;self.layer.borderWidth=1;self.layer.masksToBounds=YES;
    UIImageSymbolConfiguration *c=[UIImageSymbolConfiguration configurationWithPointSize:20 weight:UIImageSymbolWeightBold];
    self.dots=[[UIImageView alloc] initWithImage:[UIImage systemImageNamed:@"ellipsis" withConfiguration:c]];
    self.xmark=[[UIImageView alloc] initWithImage:[UIImage systemImageNamed:@"xmark" withConfiguration:c]];
    for(UIImageView *v in @[self.dots,self.xmark]){v.contentMode=UIViewContentModeCenter;v.userInteractionEnabled=NO;[self addSubview:v];}
    [self addTarget:self action:@selector(flip) forControlEvents:UIControlEventTouchUpInside];
    [self paint];
    return self;
}
- (void)layoutSubviews{[super layoutSubviews];self.dots.frame=self.bounds;self.xmark.frame=self.bounds;}
- (void)paint {
    UIColor *amber=[UIColor colorWithRed:1 green:.71 blue:.278 alpha:1];
    self.backgroundColor=JKRailClosed?[UIColor colorWithRed:.047 green:.07 blue:.078 alpha:.58]:amber;
    self.layer.borderColor=(JKRailClosed?[UIColor colorWithRed:.937 green:.957 blue:.945 alpha:.14]:UIColor.clearColor).CGColor;
    self.dots.tintColor=UIColor.whiteColor;self.xmark.tintColor=[UIColor colorWithRed:.141 green:.086 blue:0 alpha:1];
    self.dots.alpha=JKRailClosed?1:0;self.xmark.alpha=JKRailClosed?0:1;
    self.dots.transform=JKRailClosed?CGAffineTransformIdentity:CGAffineTransformMakeRotation(M_PI_2);
    self.xmark.transform=JKRailClosed?CGAffineTransformMakeRotation(-M_PI_2):CGAffineTransformIdentity;
}
- (void)flip {JKRailClosed=!JKRailClosed;[UIView animateWithDuration:.35 animations:^{[self paint];}];[self applyAnimated:YES];}
- (void)applyAnimated:(BOOL)anim {
    NSArray *subs=self.superview.subviews;NSUInteger k=0,n=0;
    for(UIView *v in subs)if([v isKindOfClass:NSClassFromString(@"JKStyleItem")]||v.tag==9161||v.tag==9162)n++;
    for(UIView *v in subs){
        if(![v isKindOfClass:NSClassFromString(@"JKStyleItem")]&&v.tag!=9161&&v.tag!=9162)continue;k++;
        UIView *item=v;NSUInteger kk=k;BOOL closed=JKRailClosed;
        void(^st)(void)=^{item.alpha=closed?0:1;item.transform=closed?CGAffineTransformConcat(CGAffineTransformMakeScale(.25,.25),CGAffineTransformMakeTranslation(0,kk*40.0)):CGAffineTransformIdentity;item.userInteractionEnabled=(item.tag==9161||item.tag==9162)?NO:!closed;};
        if(anim)[UIView animateWithDuration:.5 delay:(closed?(n-kk)*.025:(kk-1)*.045) usingSpringWithDamping:.62 initialSpringVelocity:0 options:0 animations:st completion:nil];else st();
    }
}
@end
@interface JKStyleItem:UIControl
@property(nonatomic,weak) UIView *native;
@property(nonatomic) JKKind kind;
@property(nonatomic,strong) UIImageView *glyph;
@property(nonatomic,strong) UILabel *countLabel;
@property(nonatomic,strong) NSTimer *timer;
- (void)refresh;
@end
static void JKStyleSync(UIView *native);
static void JKInstaEnsureTimer(void);
static NSArray<UIWindow *> *TT48Windows(void);
@implementation JKStyleItem
- (instancetype)initWithFrame:(CGRect)frame {
    self=[super initWithFrame:frame];if(!self)return nil;
    self.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
    self.accessibilityTraits=UIAccessibilityTraitButton;
    self.glyph=[UIImageView new];self.glyph.contentMode=UIViewContentModeScaleAspectFit;self.glyph.tintColor=UIColor.whiteColor;
    self.glyph.userInteractionEnabled=NO;
    self.glyph.layer.shadowColor=UIColor.blackColor.CGColor;self.glyph.layer.shadowOpacity=.35;self.glyph.layer.shadowRadius=2;self.glyph.layer.shadowOffset=CGSizeZero;
    self.countLabel=[UILabel new];self.countLabel.font=[UIFont systemFontOfSize:12 weight:UIFontWeightSemibold];
    self.countLabel.textColor=UIColor.whiteColor;self.countLabel.textAlignment=NSTextAlignmentCenter;self.countLabel.userInteractionEnabled=NO;
    self.countLabel.adjustsFontSizeToFitWidth=YES;self.countLabel.minimumScaleFactor=.7;
    self.countLabel.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
    [self addSubview:self.glyph];[self addSubview:self.countLabel];
    [self addTarget:self action:@selector(tapped) forControlEvents:UIControlEventTouchUpInside];
    return self;
}
- (void)layoutSubviews {
    [super layoutSubviews];
    CGFloat w=self.bounds.size.width,h=self.bounds.size.height;
    self.glyph.frame=CGRectMake((w-32)/2,MAX(0,(h-32-16)/2),32,32);
    self.countLabel.frame=CGRectMake(0,CGRectGetMaxY(self.glyph.frame)+1,w,16);
}
- (void)didMoveToWindow {
    [super didMoveToWindow];
    [self.timer invalidate];self.timer=nil;
    if(!self.window)return;
    __weak JKStyleItem *weak=self;
    self.timer=[NSTimer scheduledTimerWithTimeInterval:.4 repeats:YES block:^(NSTimer *t){
        JKStyleItem *s=weak;if(!s.window){[t invalidate];return;}[s refresh];
    }];
}
- (void)refresh {
    UIView *n=self.native;if(!n)return;
    BOOL selected=[n isKindOfClass:UIControl.class] && ((UIControl *)n).selected;
    NSString *name=@"heart",*label=@"Like";
    switch(self.kind){
        case JKKindLike:name=selected?@"heart.fill":@"heart";label=@"Like";break;
        case JKKindComment:name=@"bubble.right";label=@"Comment";break;
        case JKKindSave:name=selected?@"bookmark.fill":@"bookmark";label=@"Save";break;
        case JKKindShare:name=@"paperplane";label=@"Share";break;
        default:break;}
    UIImageSymbolConfiguration *cfg=[UIImageSymbolConfiguration configurationWithPointSize:27 weight:(JKAlaa()?UIImageSymbolWeightRegular:UIImageSymbolWeightLight)];
    self.glyph.image=[UIImage systemImageNamed:name withConfiguration:cfg];
    if(JKAlaa()){
        self.glyph.backgroundColor=nil;self.glyph.layer.cornerRadius=0;self.glyph.layer.borderWidth=0;
    } else {self.glyph.backgroundColor=nil;self.glyph.layer.cornerRadius=0;self.glyph.layer.borderWidth=0;}
    self.glyph.tintColor=(selected && self.kind==JKKindLike)?(JKAlaa()?[UIColor colorWithRed:1 green:.302 blue:.416 alpha:1]:[UIColor colorWithRed:1 green:.19 blue:.33 alpha:1]):((selected && self.kind==JKKindSave && JKAlaa())?[UIColor colorWithRed:1 green:.71 blue:.278 alpha:1]:UIColor.whiteColor);
    NSString *t=JKCountText(n,0);
    if(![self.countLabel.text isEqual:t])self.countLabel.text=t;
    BOOL can=JKCanForward(n);
    self.glyph.alpha=can?1:.45;self.countLabel.alpha=can?1:.45;self.enabled=can;
    self.accessibilityLabel=t.length?[NSString stringWithFormat:@"%@, %@",label,t]:label;
    self.hidden=n.hidden;
}
- (void)tapped {
    UIView *n=self.native;
    // Forward only to a verified native touch-up target; otherwise do nothing.
    if(n && JKCanForward(n))[(UIControl *)n sendActionsForControlEvents:UIControlEventTouchUpInside];
}
- (void)dealloc {[_timer invalidate];}
@end
static JKStyleItem *JKItemOf(UIView *native){return objc_getAssociatedObject(native,kJKItemKey);}
// Only explicitly recorded paint roots may be hidden. Never dim a native
// action container in response to a forwarding overlay (gesture-only rail safety).
static CGFloat JKStyleAlpha(UIView *view,CGFloat requested) {
    if(JKStyleOn() && objc_getAssociatedObject(view,kJKPaintAlpha))return 0;
    return requested;
}
static void JKStyleDetach(UIView *native) {
    JKStyleItem *item=JKItemOf(native);
    NSNumber *orig=objc_getAssociatedObject(native,kJKOrigAlphaKey);
    if(!item&&!orig)return;
    objc_setAssociatedObject(native,kJKItemKey,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    objc_setAssociatedObject(native,kJKOrigAlphaKey,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    [[native.superview viewWithTag:9156] removeFromSuperview];
    [item.timer invalidate];item.timer=nil;[item removeFromSuperview];
    // Even a missing/orphan replacement must release its old alpha suppression.
    [UIView performWithoutAnimation:^{native.alpha=orig?orig.doubleValue:1;}];
}
static BOOL JKStyleOwnsPaint(UIView *native){
    return JKStyleOn() && JKKindOf(native)!=JKKindNone;
}

// Per-button state, never stored in the geometry/font/attributed-string slots.
static const void *kJKPaintRec=&kJKPaintRec;
@interface JKPaintRec:NSObject
@property(nonatomic,strong) NSMapTable<UIView *,NSDictionary *> *views;
@property(nonatomic,strong) UIImageView *glyph;
@end
@implementation JKPaintRec
@end
static void JKPaintRemember(JKPaintRec *r,UIView *v){
    if([r.views objectForKey:v])return;
    NSMutableDictionary *d=[@{@"alpha":@(v.alpha),@"tint":v.tintColor?:(id)NSNull.null,
        @"bg":v.backgroundColor?:(id)NSNull.null} mutableCopy];
    [r.views setObject:d forKey:v];
}
static void JKPaintHide(JKPaintRec *r,UIView *v){
    JKPaintRemember(r,v);
    if(!objc_getAssociatedObject(v,kJKPaintAlpha))objc_setAssociatedObject(v,kJKPaintAlpha,@(v.alpha),OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    v.alpha=0;
}
static void JKPaintRestore(UIView *native){
    JKPaintRec *r=objc_getAssociatedObject(native,kJKPaintRec);if(!r)return;
    [r.glyph removeFromSuperview];
    for(UIView *v in r.views){
        NSDictionary *d=[r.views objectForKey:v];
        objc_setAssociatedObject(v,kJKPaintAlpha,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        v.alpha=[d[@"alpha"] doubleValue];id t=d[@"tint"],bg=d[@"bg"];
        v.tintColor=t==NSNull.null?nil:t;v.backgroundColor=bg==NSNull.null?nil:bg;
    }
    objc_setAssociatedObject(native,kJKPaintRec,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}
// The observed tree has both HDRUIImageView and existing TTGActionGlyph renderers.
// Record/hide paint only, not UIControls or recognizers; our glyph is inert.
static UIView *JKPaintIcon(UIView *v,NSUInteger depth){
    if(depth>5 || v.tag==9258)return nil;
    if([v isKindOfClass:UIImageView.class] && !v.hidden && v.bounds.size.width>=12 && v.bounds.size.width<=64 && v.bounds.size.height>=12)return v;
    for(UIView *c in v.subviews){UIView *i=JKPaintIcon(c,depth+1);if(i)return i;}
    return nil;
}
static void JKPaintTree(JKPaintRec *r,UIView *v,NSUInteger depth){
    if(depth>6 || v==r.glyph)return;
    if([NSStringFromClass(v.class) isEqualToString:@"TTGActionGlyph"]){JKPaintHide(r,v);return;}
    if([v isKindOfClass:UIImageView.class] && v.bounds.size.width>=12 && v.bounds.size.height>=12){JKPaintHide(r,v);return;}
    if([v isKindOfClass:UILabel.class]){JKPaintRemember(r,v);v.tintColor=UIColor.whiteColor;return;}
    for(UIView *c in [v.subviews copy])JKPaintTree(r,c,depth+1);
}
static void JKStyleSync(UIView *native) {
    JKInstaEnsureTimer();
    JKStyleDetach(native); // clean up a previous release's forwarding overlay if still present
    if(!JKStyleOn() || !native.window || !native.superview || !JKInFeed(native)){
        JKPaintRestore(native);[[native.superview viewWithTag:9156] removeFromSuperview];if(JKStyleOn())JKResult(NO,@"rail-offscreen-or-not-feed");return;
    }
    JKKind kind=JKKindOf(native);
    if(kind==JKKindNone){JKPaintRestore(native);JKResult(NO,@"rail-unidentified");return;}
    JKPaintRec *r=objc_getAssociatedObject(native,kJKPaintRec);
    UIView *icon=JKPaintIcon(native,0);
    // Reuse: do not leave old paint attached when a native icon disappears.
    if(!icon){JKPaintRestore(native);JKResult(NO,@"rail-no-icon");return;}
    if(!r){r=[JKPaintRec new];r.views=[NSMapTable weakToStrongObjectsMapTable];
        r.glyph=[UIImageView new];r.glyph.tag=9258;r.glyph.userInteractionEnabled=NO;
        r.glyph.isAccessibilityElement=NO;r.glyph.accessibilityElementsHidden=YES;
        r.glyph.contentMode=UIViewContentModeScaleAspectFit;
        objc_setAssociatedObject(native,kJKPaintRec,r,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        [native addSubview:r.glyph];
    }
    BOOL selected=[native isKindOfClass:UIControl.class] && ((UIControl *)native).selected;
    NSString *name=kind==JKKindLike?(selected?@"heart.fill":@"heart"):kind==JKKindComment?@"bubble.right":kind==JKKindSave?(selected?@"bookmark.fill":@"bookmark"):@"paperplane";
    r.glyph.image=[UIImage systemImageNamed:name withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:28 weight:UIImageSymbolWeightRegular]];
    r.glyph.tintColor=selected&&kind==JKKindLike?[UIColor colorWithRed:1 green:.302 blue:.416 alpha:1]:(selected&&kind==JKKindSave&&JKAlaa()?[UIColor colorWithRed:1 green:.71 blue:.278 alpha:1]:UIColor.whiteColor);
    CGRect f=[icon convertRect:icon.bounds toView:native];r.glyph.frame=CGRectInset(f,MAX(0,(f.size.width-32)/2),MAX(0,(f.size.height-32)/2));
    JKPaintTree(r,native,0);
    // Some builds also attach a second TTG renderer beside the button. Hide only
    // the renderer whose window rect overlaps this button, never a sibling action.
    CGRect nr=[native convertRect:native.bounds toView:native.window];
    for(UIView *v in native.superview.subviews)if([NSStringFromClass(v.class) isEqualToString:@"TTGActionGlyph"] && CGRectIntersectsRect(nr,[v convertRect:v.bounds toView:native.window]))JKPaintHide(r,v);
    [native bringSubviewToFront:r.glyph];
    JKResult(YES,native.gestureRecognizers.count?@"rail-in-place-gestures":@"rail-in-place");
}

// 54: in-place Instagram-like restyle of original views (dock, progress line, username weight).
// Nothing is replaced, re-parented or re-targeted; originals are recorded and restored.
static const void *kJKRec=&kJKRec,*kJKFontRec=&kJKFontRec,*kJKTextRec=&kJKTextRec;
static NSDictionary *JKRec(UIView *v){return objc_getAssociatedObject(v,kJKRec);}
static void JKRecSave(UIView *v){
    if(JKRec(v))return;
    objc_setAssociatedObject(v,kJKRec,@{@"frame":[NSValue valueWithCGRect:v.frame],@"bounds":[NSValue valueWithCGRect:v.bounds],@"center":[NSValue valueWithCGPoint:v.center],@"transform":[NSValue valueWithCGAffineTransform:v.transform],@"radius":@(v.layer.cornerRadius),
        @"clip":@(v.layer.masksToBounds),@"bw":@(v.layer.borderWidth),@"bc":v.layer.borderColor?(__bridge id)v.layer.borderColor:(id)NSNull.null,
        @"bg":v.backgroundColor?:(id)NSNull.null},OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}
static void JKRecRestore(UIView *v){
    NSDictionary *d=JKRec(v);if(!d)return;
    v.transform=[d[@"transform"] CGAffineTransformValue];v.bounds=[d[@"bounds"] CGRectValue];v.center=[d[@"center"] CGPointValue];v.layer.cornerRadius=[d[@"radius"] doubleValue];v.layer.masksToBounds=[d[@"clip"] boolValue];
    v.layer.borderWidth=[d[@"bw"] doubleValue];id bc=d[@"bc"];v.layer.borderColor=bc==NSNull.null?nil:(__bridge CGColorRef)bc;
    id bg=d[@"bg"];v.backgroundColor=bg==NSNull.null?nil:bg;
    objc_setAssociatedObject(v,kJKRec,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}
static void JKDecoApply(UIWindow *w,NSInteger tag,UIView *src,CGRect fr,CGFloat border,UIColor *bc,BOOL spin,BOOL on);
static UIView *JKRailStack(UIWindow *w);
static UIImage *JKSnap(UIView *v);
static const void *kJKAvaRec=&kJKAvaRec;
@interface JKAvaRec:NSObject
@property(nonatomic,weak) UIView *parent;
@property(nonatomic,strong) UIView *spacer;
@property(nonatomic) NSInteger subIndex,arrIndex;
@property(nonatomic) BOOL wasArranged;
@property(nonatomic) CGRect frame;
@property(nonatomic) CGAffineTransform transform;
@property(nonatomic,weak) UIView *author;
@property(nonatomic) CGAffineTransform authorTx;
@end
@implementation JKAvaRec
@end
static UIView *JKFindClass(UIView *root,NSString *name,NSUInteger depth){
    if([NSStringFromClass(root.class) isEqualToString:name] && root.window){
        CGRect wr=[root convertRect:root.bounds toView:root.window];BOOL visible=!root.hidden&&root.alpha>.01&&CGRectIntersectsRect(wr,root.window.bounds)&&!CGRectIsEmpty(wr);
        for(UIView *p=root.superview;visible&&p;p=p.superview)if(p.hidden||p.alpha<=.01)visible=NO;
        if(visible)return root;
    }
    if(depth>80)return nil;
    for(UIView *c in root.subviews){UIView *f=JKFindClass(c,name,depth+1);if(f)return f;}
    return nil;
}
static void JKAvatarRestore(UIView *avatar){
    JKAvaRec *r=objc_getAssociatedObject(avatar,kJKAvaRec);if(!r)return;
    objc_setAssociatedObject(avatar,kJKAvaRec,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    if(r.author)r.author.transform=r.authorTx;
    {UIWindow *w0=avatar.window;if(w0)JKDecoApply(w0,9161,nil,CGRectZero,0,nil,NO,NO);}
    for(UIView *rs in avatar.subviews)JKRecRestore(rs);
    UIView *parent=r.parent;
    if(!parent){[r.spacer removeFromSuperview];return;}
    [avatar removeFromSuperview];
    avatar.transform=CGAffineTransformIdentity;
    if(r.wasArranged && [parent isKindOfClass:UIStackView.class]){
        UIStackView *st=(UIStackView *)parent;
        [r.spacer removeFromSuperview];
        [st insertArrangedSubview:avatar atIndex:MIN((NSUInteger)MAX(r.arrIndex,0),st.arrangedSubviews.count)];
    } else {
        [parent insertSubview:avatar atIndex:MIN((NSUInteger)MAX(r.subIndex,0),parent.subviews.count)];
        [r.spacer removeFromSuperview];
    }
    avatar.frame=r.frame;avatar.transform=r.transform;
}
static void JKAvatarApply(UIView *avatar,UIWindow *window,BOOL on,int mode){
    // Keep real avatar/follow controls inside the native rail. Reparenting the
    // avatar to an RTL author container hid it outside that container's bounds,
    // bypassed eye-hide ancestry and collided with the caption on the device.
    if(objc_getAssociatedObject(avatar,kJKAvaRec))JKAvatarRestore(avatar);
    UIView *rail=window?JKRailStack(window):nil;
    [[rail viewWithTag:9161] removeFromSuperview];
    if(mode!=0)return; // follow subtree still traversed normally by JKInstaPass
    CGRect wr=[avatar convertRect:avatar.bounds toView:window];
    if(on && (!avatar.window||CGRectIsEmpty(wr)||!CGRectIntersectsRect(wr,window.bounds)))return;
    for(UIView *rs in avatar.subviews){
        CGSize z=rs.bounds.size;
        if(([rs isKindOfClass:UIButton.class]||[rs isKindOfClass:UIImageView.class]) &&
           z.width>=30 && z.width<=50 && fabs(z.width-z.height)<2){
            if(on){JKRecSave(rs);rs.layer.cornerRadius=z.width/2;rs.clipsToBounds=YES;
                rs.layer.borderWidth=2;rs.layer.borderColor=UIColor.whiteColor.CGColor;}
            else JKRecRestore(rs);
        }
    }
    if(on)JKResult(YES,@"avatar-native-rail");
}
static const void *kJKTbTx=&kJKTbTx;
static void JKDockRestore(UIView *blur){
    UIView *tbar=blur.superview;NSValue *t=tbar?objc_getAssociatedObject(tbar,kJKTbTx):nil;
    if(t){tbar.transform=t.CGAffineTransformValue;objc_setAssociatedObject(tbar,kJKTbTx,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
}
static UIImage *JKDisc(UIImage *thumb){
    CGFloat S=42;UIGraphicsImageRenderer *r=[[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(S,S)];
    return [r imageWithActions:^(UIGraphicsImageRendererContext *c){
        CGContextRef g=c.CGContext;CGRect all=CGRectMake(0,0,S,S);
        [[UIColor colorWithRed:.051 green:.071 blue:.078 alpha:1] setFill];CGContextFillEllipseInRect(g,all);
        [[UIColor colorWithWhite:1 alpha:.09] setStroke];CGContextSetLineWidth(g,.6);
        for(CGFloat in=3;in<10;in+=2.6)CGContextStrokeEllipseInRect(g,CGRectInset(all,in,in));
        CGRect ctr=CGRectInset(all,12,12);
        if(thumb){CGContextSaveGState(g);CGContextAddEllipseInRect(g,ctr);CGContextClip(g);[thumb drawInRect:ctr];CGContextRestoreGState(g);}
        else {[[UIColor colorWithRed:1 green:.71 blue:.278 alpha:1] setFill];CGContextFillEllipseInRect(g,ctr);}
        [[UIColor colorWithRed:.051 green:.071 blue:.078 alpha:1] setFill];CGContextFillEllipseInRect(g,CGRectInset(all,19,19));
    }];
}
static UIImage *JKSnap(UIView *v){
    if(v.bounds.size.width<2||v.bounds.size.height<2)return nil;
    UIGraphicsImageRenderer *r=[[UIGraphicsImageRenderer alloc] initWithSize:v.bounds.size];
    return [r imageWithActions:^(UIGraphicsImageRendererContext *c){[v.layer renderInContext:c.CGContext];}];
}
static UIView *JKRailStack(UIWindow *w){return JKFindClass(w,@"TTKFeedRightInteractionStackView",0);}
// Decorative, non-interactive copies (rendered from the native views). Real controls stay where they are.
static void JKDecoApply(UIWindow *w,NSInteger tag,UIView *src,CGRect fr,CGFloat border,UIColor *bc,BOOL spin,BOOL on){
    UIView *st=w?JKRailStack(w):nil;if(!st)return;
    UIImageView *iv=(UIImageView *)[st viewWithTag:tag];
    if(!on||!src){[iv.layer removeAllAnimations];[iv removeFromSuperview];return;}
    UIImage *im=JKSnap(src);if(tag==9162)im=JKDisc(im);if(!im && iv)return;
    if(!iv){iv=[UIImageView new];iv.tag=tag;iv.userInteractionEnabled=NO;iv.clipsToBounds=YES;iv.contentMode=UIViewContentModeScaleAspectFill;[st insertSubview:iv atIndex:0];}
    if(im)iv.image=im;iv.userInteractionEnabled=NO;if(!iv.window||!iv.layer.presentationLayer){iv.alpha=1;iv.transform=CGAffineTransformIdentity;}
    iv.frame=fr;iv.layer.cornerRadius=fr.size.width/2;iv.layer.borderWidth=(tag==9162)?0:border;iv.layer.borderColor=bc.CGColor;
    if(spin && ![iv.layer animationForKey:@"jkspin"]){CABasicAnimation *a=[CABasicAnimation animationWithKeyPath:@"transform.rotation.z"];a.fromValue=@0;a.toValue=@(M_PI*2);a.duration=6;a.repeatCount=HUGE_VALF;[iv.layer addAnimation:a forKey:@"jkspin"];}
}
static NSMutableString *JKGeo;static NSMutableDictionary *JKHits;static NSUInteger JKNodes,JKDeep;
static void JKLog(NSString *line){
    NSString *folder=NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject;if(!folder)return;
    NSString *path=[folder stringByAppendingPathComponent:@"joker_debug.txt"];
    NSString *l=[NSString stringWithFormat:@"JokerStyle %@\n",line];
    @synchronized(NSFileManager.defaultManager){
        if(![NSFileManager.defaultManager fileExistsAtPath:path])[NSData.data writeToFile:path atomically:YES];
        NSDictionary *attrs=[NSFileManager.defaultManager attributesOfItemAtPath:path error:nil];
        if([attrs fileSize]>512*1024){NSData *data=[NSData dataWithContentsOfFile:path];if(data.length>128*1024)[[data subdataWithRange:NSMakeRange(data.length-128*1024,128*1024)] writeToFile:path atomically:YES];}
        NSFileHandle *f=[NSFileHandle fileHandleForWritingAtPath:path];
        @try{[f seekToEndOfFile];[f writeData:[l dataUsingEncoding:NSUTF8StringEncoding]];[f closeFile];}@catch(NSException *e){}
    }
}


static const void *kJKTextLayoutRec=&kJKTextLayoutRec;
static void JKTextTree(UIView *v,BOOL on,NSUInteger depth){
    if(depth>10)return;
    SEL align=NSSelectorFromString(@"setTextAlignment:");
    BOOL has=[v respondsToSelector:align]&&[v respondsToSelector:NSSelectorFromString(@"textAlignment")];
    NSDictionary *r=objc_getAssociatedObject(v,kJKTextLayoutRec);
    if(on){
        if(!r){NSMutableDictionary *d=[@{@"semantic":@(v.semanticContentAttribute)} mutableCopy];
            if(has)d[@"align"]=@(((NSInteger(*)(id,SEL))objc_msgSend)(v,NSSelectorFromString(@"textAlignment")));
            objc_setAssociatedObject(v,kJKTextLayoutRec,d,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
        v.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
        if(has)((void(*)(id,SEL,NSInteger))objc_msgSend)(v,align,NSTextAlignmentLeft);
    } else if(r){
        v.semanticContentAttribute=[r[@"semantic"] integerValue];
        if(has&&r[@"align"])((void(*)(id,SEL,NSInteger))objc_msgSend)(v,align,[r[@"align"] integerValue]);
        objc_setAssociatedObject(v,kJKTextLayoutRec,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    for(UIView *c in [v.subviews copy])JKTextTree(c,on,depth+1);
}
// Physical left region, separated from the actual native rail, not from a guessed
// RTL coordinate. Keep Y unchanged. UIKit will continue to own text line layout.
static void JKLeftRegion(UIView *v,BOOL on,CGFloat left){
    if(!v.window)return;
    if(on){
        CGRect wr=[v convertRect:v.bounds toView:v.window];
        if(CGRectIsEmpty(wr)||!CGRectIntersectsRect(wr,v.window.bounds))return;
        UIView *rail=JKRailStack(v.window);CGFloat end=v.window.bounds.size.width-88;
        if(rail)end=MIN(end,CGRectGetMinX([rail convertRect:rail.bounds toView:v.window])-12);
        JKRecSave(v);
        CGRect target=CGRectMake(left,wr.origin.y,MAX(1,end-left),wr.size.height);
        // Setting bounds/center avoids frame math under a native transform.
        CGPoint c=[v.superview convertPoint:CGPointMake(CGRectGetMidX(target),CGRectGetMidY(target)) fromView:v.window];
        CGRect bounds=v.bounds;bounds.size.width=target.size.width;v.bounds=bounds;v.center=c;
        JKTextTree(v,YES,0);JKResult(YES,@"text-physical-left");
    } else {JKRecRestore(v);JKTextTree(v,NO,0);}
}
static BOOL JKSetAttributed(UIView *v,NSAttributedString *text){
    SEL sel=NSSelectorFromString(@"setAttributedText:");
    NSMethodSignature *sig=[v methodSignatureForSelector:sel];
    if(![v respondsToSelector:sel] || !sig || sig.numberOfArguments!=3 || sig.methodReturnType[0]!='v' || [sig getArgumentTypeAtIndex:2][0]!='@')return NO;
    @try{((void(*)(id,SEL,id))objc_msgSend)(v,sel,text);return YES;}@catch(__unused NSException *e){return NO;}
}
static void JKCaptionApply(UIView *v,BOOL on){
    id value=JKObject(v,@"attributedText");
    if(![value isKindOfClass:NSAttributedString.class]){if(on)JKResult(NO,@"caption-no-attributedText");return;}
    NSAttributedString *current=value,*saved=objc_getAssociatedObject(v,kJKTextRec);
    if(!on){
        // Only restore an unchanged string, never overwrite a reused video's caption.
        if([saved isKindOfClass:NSAttributedString.class] && [saved.string isEqual:current.string])JKSetAttributed(v,saved);
        objc_setAssociatedObject(v,kJKTextRec,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);return;
    }
    if(!current.length){JKResult(NO,@"caption-empty");return;}
    if(!saved || ![saved.string isEqual:current.string]){
        saved=[current copy];objc_setAssociatedObject(v,kJKTextRec,saved,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    NSMutableAttributedString *m=[saved mutableCopy];
    [saved enumerateAttribute:NSParagraphStyleAttributeName inRange:NSMakeRange(0,saved.length) options:0 usingBlock:^(NSParagraphStyle *style,NSRange range,BOOL *stop){
        NSMutableParagraphStyle *p=style?[style mutableCopy]:[NSMutableParagraphStyle new];p.alignment=NSTextAlignmentLeft;p.baseWritingDirection=NSWritingDirectionNatural;
        [m addAttribute:NSParagraphStyleAttributeName value:p range:range];
    }];
    NSRegularExpression *re=[NSRegularExpression regularExpressionWithPattern:@"#[^\\s#]+" options:0 error:nil];
    NSArray *matches=[re matchesInString:m.string options:0 range:NSMakeRange(0,m.length)];
    for(NSTextCheckingResult *r in matches)[m addAttribute:NSForegroundColorAttributeName value:[UIColor colorWithRed:1 green:.71 blue:.278 alpha:1] range:r.range];
    BOOL ok=JKSetAttributed(v,m);JKResult(ok,ok?@"caption-attributed":@"caption-no-setter");
}
static void JKInstaPass(UIView *v,BOOL on,NSUInteger depth){
    NSString *n=NSStringFromClass(v.class);JKNodes++;if(depth>JKDeep)JKDeep=depth;
    if(JKGeo&&v.window&&([n isEqualToString:@"TikTokFeedTabControl"]||[n isEqualToString:@"TTKFeedTabCornerItemView"]||[n isEqualToString:@"AWEPlayInteractionDescriptionLabel"]||[n isEqualToString:@"TTKFeedRightInteractionStackView"]||[n isEqualToString:@"TTKTabBar"])){
        CGRect r=[v convertRect:v.bounds toView:v.window];[JKGeo appendFormat:@" %@@%d,%d,%d,%d",n,(int)r.origin.x,(int)r.origin.y,(int)r.size.width,(int)r.size.height];}
    if(JKHits&&([n hasPrefix:@"TTK"]||[n hasPrefix:@"AWEPlayInteraction"]||[n hasPrefix:@"AWEFeed"]||[n hasPrefix:@"AWETabBar"]||[n hasPrefix:@"AWEUser"]))JKHits[n]=@([JKHits[n] integerValue]+1);
    if([n isEqualToString:@"AWEFeedVideoButton"])JKStyleSync(v);
    if([n isEqualToString:@"AWEPlayInteractionAuthorView"])JKLeftRegion(v,on&&JKAlaa(),12);
    if([n isEqualToString:@"AWEPlayInteractionDescriptionLabel"]||[n isEqualToString:@"AWEScrollLabelView"])JKLeftRegion(v,on&&JKAlaa(),12);
    if([n isEqualToString:@"TTKTabBarBlurView"] && [v.superview isKindOfClass:UITabBar.class]==NO && [NSStringFromClass(v.superview.class) isEqualToString:@"TTKTabBar"]==NO){}
    if([n isEqualToString:@"TTKTabBarBlurView"]){
        if(on){JKRecSave(v);CGRect o=[JKRec(v)[@"frame"] CGRectValue];
            v.frame=CGRectMake(o.origin.x+18,o.origin.y+6,o.size.width-36,o.size.height-31);
            v.layer.cornerRadius=v.frame.size.height/2;v.layer.masksToBounds=YES;
            v.layer.borderWidth=.5;v.layer.borderColor=(JKAlaa()?[UIColor colorWithRed:.937 green:.957 blue:.945 alpha:.14]:[UIColor colorWithWhite:1 alpha:.18]).CGColor;
            if(JKAlaa()){if(v.window){UIView *tbar=v.superview;
                if(tbar && !objc_getAssociatedObject(tbar,kJKTbTx))objc_setAssociatedObject(tbar,kJKTbTx,[NSValue valueWithCGAffineTransform:tbar.transform],OBJC_ASSOCIATION_RETAIN_NONATOMIC);
                CGRect wb=v.window.bounds;CGFloat topY=tbar.center.y-tbar.bounds.size.height/2;
                tbar.transform=CGAffineTransformMakeTranslation(0,wb.size.height-43-(topY+24.5));
                v.frame=[tbar convertRect:CGRectMake(16,wb.size.height-74,wb.size.width-32,62) fromView:v.window];v.layer.cornerRadius=31;}v.backgroundColor=[UIColor colorWithRed:.047 green:.07 blue:.078 alpha:.58];}}
        else {JKDockRestore(v);JKRecRestore(v);}
    } else if([n isEqualToString:@"AWEPlayInteractionMusicView"]){
        // Leave native music control visible and live. Remove old inert disc copies.
        if(v.window){UIView *st=JKRailStack(v.window);[[st viewWithTag:9162] removeFromSuperview];}
    } else if([n isEqualToString:@"AWETabBarPlusButton"]){
        UIView *ov=[v viewWithTag:9157];
        if(on && JKAlaa()){
            if(!ov){ov=[UIView new];ov.tag=9157;ov.userInteractionEnabled=NO;ov.layer.cornerRadius=14;
                UIImageView *pi=[[UIImageView alloc] initWithImage:[UIImage systemImageNamed:@"plus" withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:20 weight:UIImageSymbolWeightHeavy]]];
                pi.tag=1;pi.tintColor=[UIColor colorWithRed:.141 green:.086 blue:0 alpha:1];pi.contentMode=UIViewContentModeCenter;[ov addSubview:pi];[v addSubview:ov];}
            ov.backgroundColor=[UIColor colorWithRed:1 green:.71 blue:.278 alpha:1];
            CGFloat w=52,h=40;ov.frame=CGRectMake((v.bounds.size.width-w)/2,(v.bounds.size.height-h)/2,w,h);[ov viewWithTag:1].frame=ov.bounds;
            [v bringSubviewToFront:ov];
        } else if(ov){[ov removeFromSuperview];}
    } else if([n isEqualToString:@"TTKTabBar"] || [n isEqualToString:@"TTKFeedTabCornerItemView"] || [n isEqualToString:@"TikTokFeedTabControl"] || [n isEqualToString:@"AWEPlayInteractionFollowPromptView"]){
        BOOL al=on && JKAlaa();
        if([n isEqualToString:@"TTKTabBar"]){
            static const void *kT=&kT;
            if(al){if(!objc_getAssociatedObject(v,kT))objc_setAssociatedObject(v,kT,v.tintColor?:(id)NSNull.null,OBJC_ASSOCIATION_RETAIN_NONATOMIC);v.tintColor=[UIColor colorWithRed:1 green:.71 blue:.278 alpha:1];}
            else{id o=objc_getAssociatedObject(v,kT);if(o){v.tintColor=o==NSNull.null?nil:o;objc_setAssociatedObject(v,kT,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}}
        } else if([n isEqualToString:@"TTKFeedTabCornerItemView"]){
            JKRecRestore(v); // no dead pill backgrounds: no native 1x/speaker actions
        } else if([n isEqualToString:@"TikTokFeedTabControl"]){
            UIView *ul=[v viewWithTag:9158];UIView *sel=nil;
            if(al){NSMutableArray *q=[NSMutableArray arrayWithArray:v.subviews];
                for(NSUInteger i=0;i<q.count&&i<128;i++){UIView *c=q[i];if(c.tag==9158)continue;
                    if([c isKindOfClass:UIControl.class]&&((UIControl *)c).selected&&!c.hidden){sel=c;break;}[q addObjectsFromArray:c.subviews];}
                JKResult(sel!=nil,sel?@"tab-selected-descendant":@"tab-no-selected");}
            if(sel){if(!ul){ul=[UIView new];ul.tag=9158;ul.userInteractionEnabled=NO;ul.backgroundColor=[UIColor colorWithRed:1 green:.71 blue:.278 alpha:1];ul.layer.cornerRadius=1;[v addSubview:ul];}
                CGRect sr=[sel convertRect:sel.bounds toView:v];ul.frame=CGRectMake(CGRectGetMidX(sr)-8,CGRectGetMaxY(sr)-2,16,2);}
            else if(ul)[ul removeFromSuperview];
        } else {
            UIView *ch=[v viewWithTag:9159];
            if(al){if(!ch){ch=[UIView new];ch.tag=9159;ch.userInteractionEnabled=NO;ch.backgroundColor=[UIColor colorWithRed:.047 green:.07 blue:.078 alpha:.58];ch.layer.borderWidth=1;ch.layer.borderColor=[UIColor colorWithWhite:1 alpha:.5].CGColor;ch.layer.cornerRadius=12;
                    UILabel *l=[UILabel new];l.tag=1;l.text=@"متابعة";l.textColor=UIColor.whiteColor;l.font=[UIFont systemFontOfSize:12 weight:UIFontWeightSemibold];l.textAlignment=NSTextAlignmentCenter;[ch addSubview:l];[v addSubview:ch];}
                ch.frame=CGRectMake(0,(v.bounds.size.height-24)/2,MAX(60,v.bounds.size.width),24);[ch viewWithTag:1].frame=ch.bounds;[v bringSubviewToFront:ch];}
            else if(ch)[ch removeFromSuperview];
            if(v.window)JKAvatarApply(v,v.window,al,1);
        }
    } else if([n isEqualToString:@"AWEFeedPlayerBottomProgressBar"]){
        UIView *trk=[v viewWithTag:9160];
        if(on && JKAlaa() && v.bounds.size.width>0){
            if(!trk){trk=[UIView new];trk.tag=9160;trk.userInteractionEnabled=NO;[v insertSubview:trk atIndex:0];}
            trk.backgroundColor=[UIColor colorWithWhite:1 alpha:.28];trk.frame=CGRectMake(0,0,v.bounds.size.width,MAX(v.bounds.size.height,2));
        } else if(trk){[trk removeFromSuperview];}
        for(UIView *s in v.subviews){ if(s.tag==9160)continue;
            if(on){JKRecSave(s);s.backgroundColor=UIColor.whiteColor;} else JKRecRestore(s);
        }
    } else if([n isEqualToString:@"AWEPlayInteractionUserAvatarView"] && v.window){
        JKAvatarApply(v,v.window,on&&JKAlaa(),0);
    } else if([n isEqualToString:@"AWEUserNameLabel"]){
        for(UIView *s in v.subviews) if([s isKindOfClass:UILabel.class]){
            UILabel *l=(UILabel *)s;if(!l.text.length)continue;
            if(on && !objc_getAssociatedObject(l,kJKFontRec)){objc_setAssociatedObject(l,kJKFontRec,l.font,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
                l.font=[UIFont systemFontOfSize:l.font.pointSize weight:UIFontWeightBold];}
            else if(!on){UIFont *f=objc_getAssociatedObject(l,kJKFontRec);if([f isKindOfClass:UIFont.class]){l.font=f;objc_setAssociatedObject(l,kJKFontRec,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}}
        }
    } else if([n isEqualToString:@"AWEPlayInteractionDescriptionLabel"]){
        // This real device class is NOT UILabel. Require the public-shaped
        // attributedText getter/setter signatures before messaging it (YYLabel-like).
        JKCaptionApply(v,on&&JKAlaa());
    }
    if(depth<80 && JKNodes<8000)for(UIView *c in [v.subviews copy])JKInstaPass(c,on,depth+1);
}
static void JKInstaApplyAll(void){
    BOOL on=JKStyleOn();static NSString *last;
    JKHits=[NSMutableDictionary new];JKGeo=[NSMutableString new];JKReasons=[NSMutableDictionary new];JKStyled=JKSkipped=0;JKNodes=0;JKDeep=0;NSArray *ws=TT48Windows();
    for(UIWindow *w in ws)JKInstaPass(w,on,0);
    NSArray *keys=[JKHits.allKeys sortedArrayUsingSelector:@selector(compare:)];NSMutableString *m=[NSMutableString new];
    for(NSString *k in keys)[m appendFormat:@" %@=%@",k,JKHits[k]];
    NSString *sum=[NSString stringWithFormat:@"apply style=%ld on=%d windows=%lu nodes=%lu deepest=%lu capped=%d styled=%lu skipped=%lu reasons=%@ geo:%@ hits:%@",(long)JKStyle(),on,(unsigned long)ws.count,(unsigned long)JKNodes,(unsigned long)JKDeep,JKNodes>=8000,(unsigned long)JKStyled,(unsigned long)JKSkipped,JKReasons,JKGeo,m];
    if(![sum isEqualToString:last]){last=sum;JKLog(sum);}
    JKHits=nil;
}
static NSTimer *JKInstaTimer;
static void JKInstaEnsureTimer(void){
    if(JKInstaTimer || !JKStyleOn())return;
    JKInstaTimer=[NSTimer scheduledTimerWithTimeInterval:.5 repeats:YES block:^(NSTimer *t){
        JKInstaApplyAll();if(!JKStyleOn()){[t invalidate];JKInstaTimer=nil;}}];
}
static void JKStyleApplyAll(void) {
    JKInstaApplyAll();JKInstaEnsureTimer();

}
@interface JKStylePicker:NSObject
@property(nonatomic,weak) UILabel *note;
@end
@implementation JKStylePicker
- (void)changed:(UISegmentedControl *)s {
    if(!JKPickerAvailable(s.selectedSegmentIndex)) {
        s.selectedSegmentIndex=JKStyle();
        self.note.text=@"هالنمط لسا مو متوفر. Facebook بالنسخة 54، وJoker المستقبلي بعد الموافقة على التصميم.";
        return;
    }
    [[NSUserDefaults standardUserDefaults] setInteger:s.selectedSegmentIndex forKey:JKStyleKey];JKLog([NSString stringWithFormat:@"picker selected=%ld stored=%ld",(long)s.selectedSegmentIndex,(long)JKStyle()]);
    self.note.text=s.selectedSegmentIndex==3?@"نمط Alaa (فيض): أزرار الإعجاب والحفظ بألوان فيض، وشريط سفلي زجاجي. باقي الواجهة أصلية.":s.selectedSegmentIndex==1?@"نمط Instagram: أيقونات الإعجاب والتعليق والمشاركة والحفظ فقط. باقي الواجهة أصلية.":@"واجهة TikTok الأصلية.";
    JKStyleApplyAll();
}
@end
static void JKAddStylePicker(UIStackView *stack) {
    UILabel *title=[UILabel new];title.text=@"نمط الواجهة";title.textColor=[UIColor colorWithWhite:1 alpha:.88];
    title.font=[UIFont systemFontOfSize:13 weight:UIFontWeightSemibold];title.textAlignment=NSTextAlignmentLeft;
    [stack addArrangedSubview:title];
    UISegmentedControl *seg=[[UISegmentedControl alloc] initWithItems:@[@"TikTok",JK_INSTA_SHIPPED?@"Instagram":@"Instagram · قريباً",@"Facebook · قريباً",@"Alaa"]];
    seg.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
    seg.selectedSegmentIndex=JKPickerAvailable(JKStyle())?JKStyle():0;
    seg.accessibilityLabel=@"نمط الواجهة";
    [seg.heightAnchor constraintEqualToConstant:42].active=YES;
    seg.backgroundColor=[UIColor colorWithWhite:1 alpha:.08];
    seg.selectedSegmentTintColor=[UIColor colorWithRed:.80 green:.74 blue:.60 alpha:.32];
    [seg setTitleTextAttributes:@{NSForegroundColorAttributeName:UIColor.whiteColor,NSFontAttributeName:[UIFont systemFontOfSize:11 weight:UIFontWeightSemibold]} forState:UIControlStateNormal];
    seg.layer.cornerRadius=12;seg.clipsToBounds=YES;
    JKStylePicker *picker=[JKStylePicker new];
    UILabel *note=[UILabel new];note.numberOfLines=0;note.font=[UIFont systemFontOfSize:12];note.textColor=[UIColor colorWithWhite:1 alpha:.74];
    note.textAlignment=NSTextAlignmentLeft;note.text=@"Instagram يغيّر أزرار الإعجاب والتعليق والمشاركة والحفظ فقط. Facebook وJoker غير متوفرين بعد.";
    picker.note=note;objc_setAssociatedObject(seg,"jk.picker",picker,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    [seg addTarget:picker action:@selector(changed:) forControlEvents:UIControlEventValueChanged];
    [stack addArrangedSubview:seg];[stack addArrangedSubview:note];
}

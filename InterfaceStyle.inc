// Joker interface style (52). Style 1 = Instagram Reels-like right action rail.
// Only AWEFeedVideoButton-style native action controls that are positively identified
// (identity keywords) AND have a UIControl touch-up target are replaced. Everything else
// stays native. Taps are forwarded to the native control; counts are read from its label.
static NSString *const JKStyleKey=@"tt.interface.style";
static const void *kJKItemKey=&kJKItemKey,*kJKOrigAlphaKey=&kJKOrigAlphaKey;
typedef NS_ENUM(NSInteger,JKKind){JKKindNone=0,JKKindLike,JKKindComment,JKKindSave,JKKindShare};
static NSInteger JKStyle(void){return [[NSUserDefaults standardUserDefaults] integerForKey:JKStyleKey];}
// 0 TikTok, 1 Instagram Reels are shipped. 2 Facebook (53) and 3 Joker futuristic are not available yet.
static BOOL JKStyleAvailable(NSInteger style){return style==0 || style==1;}
// Release gate: Instagram rail ships in 53 (picker offers TikTok + Instagram; Facebook 54, Joker style after mockup approval).
#ifndef JK_INSTA_SHIPPED
#define JK_INSTA_SHIPPED 1
#endif
static BOOL JKPickerAvailable(NSInteger style){return style==0 || (JK_INSTA_SHIPPED && style==1);}
static BOOL JKStyleOn(void){NSInteger s=JKStyle();return s==1 && JKStyleAvailable(s);}
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
@interface JKStyleItem:UIControl
@property(nonatomic,weak) UIView *native;
@property(nonatomic) JKKind kind;
@property(nonatomic,strong) UIImageView *glyph;
@property(nonatomic,strong) UILabel *countLabel;
@property(nonatomic,strong) NSTimer *timer;
- (void)refresh;
@end
static void JKStyleSync(UIView *native);
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
    UIImageSymbolConfiguration *cfg=[UIImageSymbolConfiguration configurationWithPointSize:27 weight:UIImageSymbolWeightLight];
    self.glyph.image=[UIImage systemImageNamed:name withConfiguration:cfg];
    self.glyph.tintColor=(selected && self.kind==JKKindLike)?[UIColor colorWithRed:1 green:.19 blue:.33 alpha:1]:UIColor.whiteColor;
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
// Called from the UIView setAlpha hook: a replaced native action stays invisible while our item is attached.
static CGFloat JKStyleAlpha(UIView *view,CGFloat requested) {
    JKStyleItem *item=JKItemOf(view);
    if(!item || item.superview!=view.superview || !JKStyleOn())return requested;
    if(requested>0)objc_setAssociatedObject(view,kJKOrigAlphaKey,@(requested),OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    return 0;
}
static void JKStyleDetach(UIView *native) {
    JKStyleItem *item=JKItemOf(native);if(!item)return;
    objc_setAssociatedObject(native,kJKItemKey,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    [item.timer invalidate];item.timer=nil;[item removeFromSuperview];
    NSNumber *orig=objc_getAssociatedObject(native,kJKOrigAlphaKey);
    objc_setAssociatedObject(native,kJKOrigAlphaKey,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    [UIView performWithoutAnimation:^{native.alpha=orig?orig.doubleValue:1;}];
}
static void JKStyleSync(UIView *native) {
    if(!JKStyleOn() || !native.window || !native.superview || !JKInFeed(native)){JKStyleDetach(native);return;}
    JKKind kind=JKKindOf(native);
    if(kind==JKKindNone || !JKCanForward(native)){JKStyleDetach(native);return;}
    JKStyleItem *item=JKItemOf(native);
    if(!item || item.superview!=native.superview || item.kind!=kind) {
        JKStyleDetach(native);
        item=[[JKStyleItem alloc] initWithFrame:native.frame];item.native=native;item.kind=kind;
        objc_setAssociatedObject(native,kJKOrigAlphaKey,@(native.alpha>0?native.alpha:1),OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        objc_setAssociatedObject(native,kJKItemKey,item,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        [native.superview insertSubview:item aboveSubview:native];
    }
    item.frame=native.frame;item.transform=native.transform;[item refresh];
    if(native.alpha!=0)native.alpha=0; // setAlpha hook records the real value first
}
static void JKStyleApplyAll(void) {
    Class cls=NSClassFromString(@"AWEFeedVideoButton");if(!cls)return;
    for(UIWindow *w in TT48Windows()) {
        NSMutableArray<UIView *> *q=[NSMutableArray arrayWithObject:w];
        for(NSUInteger i=0;i<q.count && i<2600;i++) {
            UIView *v=q[i];if([v isKindOfClass:JKStyleItem.class])continue;
            if([v isKindOfClass:cls])JKStyleSync(v);
            [q addObjectsFromArray:v.subviews];
        }
    }
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
    [[NSUserDefaults standardUserDefaults] setInteger:s.selectedSegmentIndex forKey:JKStyleKey];
    self.note.text=s.selectedSegmentIndex==1?@"نمط Instagram: أيقونات الإعجاب والتعليق والمشاركة والحفظ فقط. باقي الواجهة أصلية.":@"واجهة TikTok الأصلية.";
    JKStyleApplyAll();
}
@end
static void JKAddStylePicker(UIStackView *stack) {
    UILabel *title=[UILabel new];title.text=@"نمط الواجهة";title.textColor=[UIColor colorWithWhite:1 alpha:.88];
    title.font=[UIFont systemFontOfSize:13 weight:UIFontWeightSemibold];title.textAlignment=NSTextAlignmentLeft;
    [stack addArrangedSubview:title];
    UISegmentedControl *seg=[[UISegmentedControl alloc] initWithItems:@[@"TikTok",JK_INSTA_SHIPPED?@"Instagram":@"Instagram · قريباً",@"Facebook · قريباً",@"Joker · قريباً"]];
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

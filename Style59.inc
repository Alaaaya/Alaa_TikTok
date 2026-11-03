// Alaa/فيض reference geometry. Native actions and navigation stay native.
// Only public AVPlayer/AVPlayerLayer methods are used; no private selectors.
#import <AVFoundation/AVFoundation.h>
static const void *kJK59DockState=&kJK59DockState,*kJK59RailState=&kJK59RailState,*kJK59RailGeometry=&kJK59RailGeometry,*kJK59PlayerState=&kJK59PlayerState;
static UIColor *JK59Amber(void){return [UIColor colorWithRed:1 green:181/255.0 blue:71/255.0 alpha:1];}
static UIColor *JK59Glass(void){return [UIColor colorWithRed:12/255.0 green:18/255.0 blue:20/255.0 alpha:.58];}
static BOOL JK59Visible(UIView *v){
    if(!v.window||v.hidden||v.alpha<.01)return NO;
    for(UIView *p=v.superview;p;p=p.superview)if(p.hidden||p.alpha<.01)return NO;
    return !CGRectIsEmpty(CGRectIntersection([v convertRect:v.bounds toView:v.window],v.window.bounds));
}
// Restore paint state separately from geometry/attributed text records.
static void JK59HidePaint(UIView *v,NSMapTable *records){
    if(![records objectForKey:v])[records setObject:@(v.alpha) forKey:v];
    objc_setAssociatedObject(v,kJKDockPaintAlpha,@YES,OBJC_ASSOCIATION_RETAIN_NONATOMIC);v.alpha=0;
}
static void JK59ReleasePaint(NSMapTable *records){
    for(UIView *v in records){objc_setAssociatedObject(v,kJKDockPaintAlpha,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);v.alpha=[[records objectForKey:v] doubleValue];}
}
static void JK59Source(UIView *v,UIImageView **icon,UILabel **label,NSUInteger depth){
    if(depth>8||v.tag==9359)return;
    if([v isKindOfClass:UILabel.class] && ((UILabel *)v).text.length && !v.hidden){if(!*label)*label=(UILabel *)v;return;}
    if([v isKindOfClass:UIImageView.class] && ((UIImageView *)v).image && !v.hidden && v.alpha>.01){if(!*icon)*icon=(UIImageView *)v;return;}
    for(UIView *s in [v.subviews copy])JK59Source(s,icon,label,depth+1);
}
@interface JK59DockRecord:NSObject
@property(nonatomic,strong) NSMapTable *paint;
@property(nonatomic,strong) UIImage *icon;
@property(nonatomic,copy) NSString *text;
@property(nonatomic,strong) UIView *decoration;
@end
@implementation JK59DockRecord
@end
static void JK59Button(UIView *b,CGRect frame,BOOL plus,BOOL on,BOOL selected){
    JK59DockRecord *r=objc_getAssociatedObject(b,kJK59DockState);
    if(!on){
        if(r){[r.decoration removeFromSuperview];JK59ReleasePaint(r.paint);JKRecRestore(b);objc_setAssociatedObject(b,kJK59DockState,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
        return;
    }
    UIImageView *icon=nil;UILabel *label=nil;
    if(!r){JK59Source(b,&icon,&label,0);
        // Preserve actual native destination labels; never label Inbox as Settings.
        NSString *text=label.text.length?label.text:b.accessibilityLabel;
        if(!plus && (!icon.image || !text.length)){JKResult(NO,@"dock-no-native-label-or-icon");return;}
        r=[JK59DockRecord new];r.paint=[NSMapTable weakToStrongObjectsMapTable];r.icon=icon.image;r.text=text;
        r.decoration=[UIView new];r.decoration.tag=9359;r.decoration.userInteractionEnabled=NO;r.decoration.accessibilityElementsHidden=YES;
        objc_setAssociatedObject(b,kJK59DockState,r,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        JKRecSave(b);
    }
    for(UIView *v in [b.subviews copy])if(v!=r.decoration)JK59HidePaint(v,r.paint);
    if(r.decoration.superview!=b)[b addSubview:r.decoration];
    b.transform=CGAffineTransformIdentity;b.frame=frame;
    UIView *d=r.decoration;d.frame=b.bounds;d.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
    for(UIView *v in [d.subviews copy])[v removeFromSuperview];
    UIColor *ink=selected?JK59Amber():[UIColor colorWithRed:239/255.0 green:244/255.0 blue:241/255.0 alpha:.72];
    UIImageView *im=[[UIImageView alloc] initWithImage:plus?[UIImage systemImageNamed:@"plus" withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:22 weight:UIImageSymbolWeightBold]]:[r.icon imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate]];
    im.userInteractionEnabled=NO;im.contentMode=UIViewContentModeScaleAspectFit;im.tintColor=plus?[UIColor colorWithRed:36/255.0 green:22/255.0 blue:0 alpha:1]:ink;
    if(plus){
        UIView *pill=[[UIView alloc] initWithFrame:CGRectMake((frame.size.width-54)/2,12,54,38)];pill.userInteractionEnabled=NO;pill.backgroundColor=JK59Amber();pill.layer.cornerRadius=13;
        im.frame=CGRectMake(16,8,22,22);[pill addSubview:im];[d addSubview:pill];
    }else{
        im.frame=CGRectMake((frame.size.width-23)/2,12,23,23);[d addSubview:im];
        UILabel *l=[[UILabel alloc] initWithFrame:CGRectMake(0,37,frame.size.width,14)];l.text=r.text;l.font=[UIFont systemFontOfSize:10.5 weight:UIFontWeightMedium];l.textColor=ink;l.textAlignment=NSTextAlignmentCenter;l.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
        l.adjustsFontSizeToFitWidth=YES;l.minimumScaleFactor=.75;l.userInteractionEnabled=NO;[d addSubview:l];
    }
    JKResult(YES,@"dock-native-in-place");
}
// The device tree reports every TTKTabBarButton selected=0; the known
// noninteractive TTKFakeTabBar exposes the actual four-item selectedItem.
// Read only public UITabBar APIs, never guess a private selected-index getter.
static UITabBar *JK59FakeBar(UIView *bar){
    NSMutableArray *q=[NSMutableArray arrayWithObject:bar.window];UITabBar *found=nil;
    for(NSUInteger i=0;i<q.count&&i<5000;i++){
        UIView *v=q[i];
        if([NSStringFromClass(v.class) isEqualToString:@"TTKFakeTabBar"] && [v isKindOfClass:UITabBar.class] && !v.hidden && !CGRectIsEmpty(CGRectIntersection([v convertRect:v.bounds toView:bar.window],bar.window.bounds))){
            if(found)return nil;found=(UITabBar *)v;
        }
        [q addObjectsFromArray:[v.subviews copy]];
    }
    return found;
}
static void JK59Dock(UIView *bar,BOOL on){
    NSMutableArray *buttons=[NSMutableArray new];
    for(UIView *v in [bar.subviews copy]){
        NSString *n=NSStringFromClass(v.class);
        if([n isEqualToString:@"TTKTabBarButton"]||[n isEqualToString:@"AWETabBarPlusButton"])[buttons addObject:v];
    }
    [buttons sortUsingComparator:^NSComparisonResult(UIView *a,UIView *b){
        NSDictionary *ra=JKRec(a),*rb=JKRec(b);CGFloat ax=ra?[ra[@"frame"] CGRectValue].origin.x:a.frame.origin.x,bx=rb?[rb[@"frame"] CGRectValue].origin.x:b.frame.origin.x;
        return ax<bx?NSOrderedAscending:ax>bx?NSOrderedDescending:NSOrderedSame;
    }];
    NSMapTable *bg=objc_getAssociatedObject(bar,kJK59DockState);
    if(!on){
        for(UIView *b in buttons)JK59Button(b,CGRectZero,NO,NO,NO);
        [[bar viewWithTag:9358] removeFromSuperview];
        if(bg){JK59ReleasePaint(bg);JKRecRestore(bar);objc_setAssociatedObject(bar,kJK59DockState,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
        return;
    }
    // Evidence is exactly five native controls, including a real plus action.
    if(buttons.count!=5 || ![NSStringFromClass(((UIView *)buttons[2]).class) isEqualToString:@"AWETabBarPlusButton"]){JKResult(NO,@"dock-native-five-controls-not-found");return;}
    if(!bg){bg=[NSMapTable weakToStrongObjectsMapTable];objc_setAssociatedObject(bar,kJK59DockState,bg,OBJC_ASSOCIATION_RETAIN_NONATOMIC);JKRecSave(bar);}
    for(UIView *v in [bar.subviews copy])if(v.tag!=9358 && ![buttons containsObject:v])JK59HidePaint(v,bg);
    CGRect target=CGRectMake(16,bar.window.bounds.size.height-74,bar.window.bounds.size.width-32,62);
    bar.transform=CGAffineTransformIdentity;bar.frame=[bar.superview convertRect:target fromView:bar.window];bar.layer.cornerRadius=31;bar.layer.masksToBounds=YES;bar.backgroundColor=JK59Glass();
    bar.layer.borderWidth=1;bar.layer.borderColor=[UIColor colorWithRed:239/255.0 green:244/255.0 blue:241/255.0 alpha:.14].CGColor;
    UIVisualEffectView *glass=(UIVisualEffectView *)[bar viewWithTag:9358];
    if(!glass){glass=[[UIVisualEffectView alloc] initWithEffect:[UIBlurEffect effectWithStyle:UIBlurEffectStyleDark]];glass.tag=9358;glass.userInteractionEnabled=NO;[bar insertSubview:glass atIndex:0];}
    glass.frame=bar.bounds;glass.layer.cornerRadius=31;glass.clipsToBounds=YES;
    UITabBar *fake=JK59FakeBar(bar);NSUInteger selectedIndex=NSNotFound;
    if(fake.items.count==4 && fake.selectedItem){NSUInteger j=[fake.items indexOfObjectIdenticalTo:fake.selectedItem];if(j<4)selectedIndex=j<2?j:j+1;}
    if(fake && !fake.userInteractionEnabled)JK59HidePaint(fake,bg);
    CGFloat unit=(bar.bounds.size.width-12)/5.2,x=6;
    for(NSUInteger i=0;i<buttons.count;i++){
        UIView *b=buttons[i];BOOL selected=selectedIndex!=NSNotFound?i==selectedIndex:([b isKindOfClass:UIControl.class]&&((UIControl *)b).selected);
        CGFloat width=unit*(i==2?1.2:1);JK59Button(b,CGRectMake(x,0,width,62),i==2,YES,selected);x+=width;
    }
}
// Only one mostly-visible known TTPlayerView may supply a bridge. No KVC,
// guessed getters, private player APIs, offscreen cells or global player lookup.
static void JK59CollectPlayers(UIView *root,NSMutableArray *views,NSUInteger depth){
    if(depth>80||views.count>8)return;
    if([NSStringFromClass(root.class) isEqualToString:@"TTPlayerView"] && JK59Visible(root)){
        CGRect r=[root convertRect:root.bounds toView:root.window],hit=CGRectIntersection(r,root.window.bounds);
        if(r.size.width>0 && r.size.height>0 && hit.size.width*hit.size.height/(r.size.width*r.size.height)>.8 && CGRectContainsPoint(r,CGPointMake(CGRectGetMidX(root.window.bounds),CGRectGetMidY(root.window.bounds))))[views addObject:root];
    }
    for(UIView *v in [root.subviews copy])JK59CollectPlayers(v,views,depth+1);
}
static void JK59Layers(CALayer *layer,CALayer *root,NSMutableArray *players,NSUInteger depth){
    if(depth>12||layer.hidden||layer.opacity<.01)return;
    CGRect rect=[layer convertRect:layer.bounds toLayer:root];
    if(CGRectIsEmpty(rect)||!CGRectIntersectsRect(rect,root.bounds))return;
    if([layer isKindOfClass:AVPlayerLayer.class] && [layer respondsToSelector:@selector(player)]){
        AVPlayer *p=((AVPlayerLayer *)layer).player;
        if([p isKindOfClass:AVPlayer.class] && [p respondsToSelector:@selector(rate)] && [p respondsToSelector:@selector(setRate:)] && [p respondsToSelector:@selector(isMuted)] && [p respondsToSelector:@selector(setMuted:)] && ![players containsObject:p])[players addObject:p];
    }
    for(CALayer *l in [layer.sublayers copy])JK59Layers(l,root,players,depth+1);
}
static AVPlayer *JK59ActivePlayer(UIWindow *w){
    NSMutableArray *views=[NSMutableArray new];JK59CollectPlayers(w,views,0);
    if(views.count!=1){JKResult(NO,views.count?@"player-ambiguous-visible-feeds":@"player-no-visible-TTPlayerView");return nil;}
    NSMutableArray *players=[NSMutableArray new];UIView *v=views.firstObject;JK59Layers(v.layer,v.layer,players,0);
    if(players.count!=1){JKResult(NO,players.count?@"player-ambiguous-layers":@"player-no-public-AVPlayerLayer");return nil;}
    AVPlayer *p=players.firstObject;
    if(p.status!=AVPlayerStatusReadyToPlay || p.currentItem.status!=AVPlayerItemStatusReadyToPlay){JKResult(NO,@"player-not-ready");return nil;}
    return p;
}
static void JK59RememberPlayer(AVPlayer *p){
    if(!objc_getAssociatedObject(p,kJK59PlayerState))objc_setAssociatedObject(p,kJK59PlayerState,[@{@"item":p.currentItem,@"rate":@(p.rate),@"mute":@(p.muted)} mutableCopy],OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}
static BOOL JK59Rate(UIWindow *w,float value){
    AVPlayer *p=JK59ActivePlayer(w);if(!p || p.rate<=0 || !(value==.5f||value==1.f||value==1.5f||value==2.f))return NO;
    // Don't resume paused feeds. This UI only changes an already playing item.
    JK59RememberPlayer(p);p.rate=value;
    BOOL ok=fabsf(p.rate-value)<.01f;
    NSMutableDictionary *r=objc_getAssociatedObject(p,kJK59PlayerState);r[@"lastRate"]=@(p.rate);
    JKResult(ok,ok?@"player-rate-readback-ok":@"player-rate-readback-failed");return ok;
}
static BOOL JK59Mute(UIWindow *w){
    AVPlayer *p=JK59ActivePlayer(w);if(!p)return NO;
    JK59RememberPlayer(p);BOOL next=!p.muted;p.muted=next;
    BOOL ok=p.muted==next;NSMutableDictionary *r=objc_getAssociatedObject(p,kJK59PlayerState);r[@"lastMute"]=@(p.muted);
    JKResult(ok,ok?@"player-mute-readback-ok":@"player-mute-readback-failed");return ok;
}
@interface JK59PlayerButton:UIButton
@property(nonatomic) BOOL speed;
@property(nonatomic,weak) AVPlayer *boundPlayer;
- (void)refresh;
@end
@implementation JK59PlayerButton
- (instancetype)initWithFrame:(CGRect)f{
    if((self=[super initWithFrame:f])){
        self.backgroundColor=JK59Glass();self.layer.cornerRadius=20;self.layer.borderWidth=1;self.layer.borderColor=[UIColor colorWithWhite:1 alpha:.14].CGColor;
        self.titleLabel.font=[UIFont systemFontOfSize:13 weight:UIFontWeightSemibold];[self setTitleColor:UIColor.whiteColor forState:UIControlStateNormal];self.tintColor=UIColor.whiteColor;
        self.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
        [self addTarget:self action:@selector(press) forControlEvents:UIControlEventTouchUpInside];
    }return self;
}
- (void)press{
    AVPlayer *p=JK59ActivePlayer(self.window);
    if(!p || p!=self.boundPlayer){[self refresh];return;} // stale cell cannot receive a tap
    if(self.speed){float r=p.rate,next=r<.75?1:r<1.25?1.5:r<1.75?2:.5;JK59Rate(self.window,next);}
    else JK59Mute(self.window);
    [self refresh];
}
- (void)refresh{
    AVPlayer *p=JK59ActivePlayer(self.window);self.boundPlayer=p;self.enabled=p && (!self.speed||p.rate>0);
    if(self.speed){[self setTitle:p?[NSString stringWithFormat:@"%g×",p.rate]:@"" forState:UIControlStateNormal];}
    else{[self setImage:[UIImage systemImageNamed:p.muted?@"speaker.slash":@"speaker.wave.2" withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:20 weight:UIImageSymbolWeightRegular]] forState:UIControlStateNormal];}
    self.accessibilityLabel=self.speed?@"سرعة التشغيل":@"كتم الصوت";
    self.accessibilityValue=p?(self.speed?[NSString stringWithFormat:@"%g",p.rate]:(p.muted?@"Muted":@"Sound on")):nil;
    self.backgroundColor=(!self.speed && p.muted)?JK59Amber():JK59Glass();self.tintColor=(!self.speed && p.muted)?[UIColor colorWithRed:.141 green:.086 blue:0 alpha:1]:UIColor.whiteColor;
}
@end
// Collapse direct native rail children, including avatar + nested follow,
// music + counts, not merely our paint. A separate sibling trigger stays live.
static void JKRailRestore(UIView *rail){
    NSMapTable *states=objc_getAssociatedObject(rail,kJK59RailState);
    for(UIView *v in states){
        NSDictionary *r=[states objectForKey:v];objc_setAssociatedObject(v,kJKCollapseAlpha,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        [v.layer removeAllAnimations];v.transform=[r[@"transform"] CGAffineTransformValue];v.alpha=[r[@"alpha"] doubleValue];v.hidden=[r[@"hidden"] boolValue];v.userInteractionEnabled=[r[@"interaction"] boolValue];
    }
    objc_setAssociatedObject(rail,kJK59RailState,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    NSValue *t=objc_getAssociatedObject(rail,kJK59RailGeometry);if(t){rail.transform=t.CGAffineTransformValue;objc_setAssociatedObject(rail,kJK59RailGeometry,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
}
// 60 owner change: collapse ALL exact feed chrome, not just rail children.
// Snapshot each native view independently. Never touch shared ancestors/video.
static void JKRailApply(UIWindow *w,BOOL animated){
    if(!w)return;
    // Clean any 59 per-child spring state first, before taking 60 snapshots.
    NSMutableArray *q=[NSMutableArray arrayWithObject:w];
    for(NSUInteger i=0;i<q.count&&i<8000;i++){UIView *v=q[i];if(objc_getAssociatedObject(v,kJK59RailState))JKRailRestore(v);[q addObjectsFromArray:[v.subviews copy]];}
    UIView *rail=JKRailStack(w);BOOL navigationChild=NO;
    for(UIView *v in q)if(([NSStringFromClass(v.class) isEqualToString:@"TTKTabBar"]||[v isKindOfClass:UITabBar.class])&&[v isDescendantOfView:rail])navigationChild=YES;
    // Preserve 59 expanded rail geometry without translating a navigation ancestor.
    if(rail&&!navigationChild){
        NSValue *g=objc_getAssociatedObject(rail,kJK59RailGeometry);
        if(!g){g=[NSValue valueWithCGAffineTransform:rail.transform];objc_setAssociatedObject(rail,kJK59RailGeometry,g,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
        rail.transform=g.CGAffineTransformValue;CGRect rr=[rail convertRect:rail.bounds toView:w];
        rail.transform=CGAffineTransformConcat(g.CGAffineTransformValue,CGAffineTransformMakeTranslation(0,-MAX(0,CGRectGetMaxY(rr)-(w.bounds.size.height-186))));
    }
    if(JK60Active())JK60Tree(w);else JK60Restore();
    JKAlaaToggle *toggle=(id)[w viewWithTag:9156];[toggle paint];
    JKResult(YES,JKRailClosed?@"all-feed-chrome-collapsed":@"all-feed-chrome-restored");
}
// Window-level floating trigger must never sit above a native presented sheet,
// settings page or system prompt. This changes neither eye nor collapse state.
static BOOL JK60ModalCovered(UIViewController *root){return root.presentedViewController!=nil;}
static void JK59Window(UIWindow *w,BOOL on){
    UIView *rail=JKRailStack(w);JKAlaaToggle *toggle=(JKAlaaToggle *)[w viewWithTag:9156];
    if(!on){
        JK60Restore();
        // Find rails even if hidden by eye. Restore is independent of style selection.
        NSMutableArray *q=[NSMutableArray arrayWithObject:w];
        for(NSUInteger i=0;i<q.count&&i<8000;i++){UIView *v=q[i];if(objc_getAssociatedObject(v,kJK59RailState)||objc_getAssociatedObject(v,kJK59RailGeometry))JKRailRestore(v);[q addObjectsFromArray:[v.subviews copy]];}
        [toggle removeFromSuperview];
        for(NSInteger tag=9360;tag<=9361;tag++)[[w viewWithTag:tag] removeFromSuperview];
        return;
    }
    // If eye hides the whole native rail, keep the trigger/top controls hidden too,
    // without overwriting the pre-collapse native child states.
    if(rail){
        // A direct window child avoids feed scrolling/clipping while leaving every
        // point outside the actual 48pt button to native hit testing.
        if(!toggle)toggle=[[JKAlaaToggle alloc] initWithFrame:CGRectMake(0,0,48,48)];
        if(toggle.superview!=w){[toggle removeFromSuperview];[w addSubview:toggle];}
        [toggle placeFloating];
        BOOL covered=TT50EyeActive||JK60ModalCovered(w.rootViewController);
        toggle.hidden=covered;toggle.userInteractionEnabled=!covered;JKRailApply(w,NO);
    } else if(toggle){toggle.hidden=YES;toggle.userInteractionEnabled=NO;}
    AVPlayer *player=JK59ActivePlayer(w);
    for(NSInteger i=0;i<2;i++){
        JK59PlayerButton *b=(JK59PlayerButton *)[w viewWithTag:9360+i];
        if(!player || TT50EyeActive){[b removeFromSuperview];continue;} // no inert controls
        if(!b){b=[[JK59PlayerButton alloc] initWithFrame:CGRectZero];b.tag=9360+i;b.speed=i==0;[w addSubview:b];}
        // Exact reference top12; controls stay in the two outer corners, not the center notch.
        CGFloat y=12,width=i==0?54:40;
        b.frame=CGRectMake(i==0?16:w.bounds.size.width-16-width,y,width,40);[b refresh];[w bringSubviewToFront:b];
    }
}

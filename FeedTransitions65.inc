// Joker 65: optional vertical-feed page transitions. Visual transforms only.
// Observes the live AWENewFeedTableView (cells below its paging viewport). Native
// paging, delegates, gestures, insets, offsets, playback and cell parents are never
// touched. Each pass restores the exact native layer state, then composes a stable
// baseline * effect once. No display link; UIKit's own layout passes drive updates.
static NSString *const JK65Key=@"tt65.feedTransition";
static const double JK65Perspective=900.0;
typedef struct {double desired,tz,rotX,scale,alpha,z;BOOL pinned;} JK65Out;
static NSArray<NSString *> *JK65Names(void){return @[@"تيك توك الأصلي",@"مكعب ثلاثي الأبعاد",@"بطاقات متراكمة",@"تلاشي مع تكبير",@"بارالاكس",@"قلب"];}
static NSInteger JK65Style(void){
    id n=[NSUserDefaults.standardUserDefaults objectForKey:JK65Key];
    if(![n isKindOfClass:NSNumber.class])return 0;
    double v=[n doubleValue];return isfinite(v)&&v==floor(v)&&v>=0&&v<6?(NSInteger)v:0;
}
static NSInteger JK65Effective(NSInteger style,BOOL reduce){return reduce?0:style;}
// p = signed page distance of a cell from the viewport (+ below). forward = the
// feed is moving to higher offsets. native position of the cell centre is p*H
// from the viewport centre. desired = wanted centre offset (viewport coords).
static JK65Out JK65Params(NSInteger style,double p,BOOL forward,double H){
    JK65Out o={p*H,0,0,1,1,0,NO};
    if(!isfinite(p)||style<=0||style>5)return o;
    double a=fabs(p);if(a>1)a=1;
    BOOL incoming=forward?p>0:p<0;
    switch(style){
        case 1:{double phi=p*M_PI/2;if(fabs(p)>1)phi=(p>0?1:-1)*M_PI/2;
            o.desired=H/2*sin(phi);o.tz=-H/2+H/2*cos(phi);o.rotX=-phi;break;}
        case 2:
            if(incoming){o.z=2;}else{o.desired=0;o.scale=1-.1*a;o.alpha=1-.35*a;o.pinned=YES;}
            break;
        case 3:o.desired=0;o.scale=1-.15*a;o.alpha=1-a;o.pinned=YES;break;
        case 4:
            if(incoming){o.z=2;}else{o.desired=p*H*.5;o.alpha=1-.4*a;}
            break;
        case 5:o.desired=0;o.rotX=-p*M_PI;o.alpha=a<.5?1:0;o.pinned=YES;break;
    }
    return o;
}
static CATransform3D JK65Matrix(NSInteger style,JK65Out o,double p,double H,CATransform3D base){
    double c=p*H;
    CATransform3D m=CATransform3DMakeScale(o.scale,o.scale,1);
    if(o.rotX!=0)m=CATransform3DConcat(m,CATransform3DMakeRotation(o.rotX,1,0,0));
    m=CATransform3DConcat(m,CATransform3DMakeTranslation(0,o.desired,o.tz));
    if(style==1||style==5){CATransform3D P=CATransform3DIdentity;P.m34=-1.0/JK65Perspective;m=CATransform3DConcat(m,P);}
    m=CATransform3DConcat(m,CATransform3DMakeTranslation(0,-c,0));
    return CATransform3DConcat(base,m);
}
static NSMapTable *JK65Records(void){static NSMapTable *t;static dispatch_once_t o;dispatch_once(&o,^{t=[NSMapTable weakToStrongObjectsMapTable];});return t;}
// Owner table per cell (weak both sides): restore never relies on the cell's direct superview.
static NSMapTable *JK65Owners(void){static NSMapTable *t;static dispatch_once_t o;dispatch_once(&o,^{t=[NSMapTable weakToWeakObjectsMapTable];});return t;}
// record = @[base transform, applied transform, base opacity, applied opacity, base zPosition, applied z]
static void JK65RestoreCell(UIView *cell,BOOL force){
    NSArray *r=[JK65Records() objectForKey:cell];if(!r)return;
    CATransform3D applied=[r[1] CATransform3DValue];
    // Unwind each property only if it still holds what we applied; a native change to any one is kept.
    if(CATransform3DEqualToTransform(cell.layer.transform,applied))cell.layer.transform=[r[0] CATransform3DValue];
    if(fabs(cell.layer.opacity-[r[3] floatValue])<1e-4)cell.layer.opacity=[r[2] floatValue];
    if(cell.layer.zPosition==[r[5] doubleValue])cell.layer.zPosition=[r[4] doubleValue];
    [JK65Records() removeObjectForKey:cell];[JK65Owners() removeObjectForKey:cell];
}
static void JK65RestoreTable(UIView *table){
    for(UIView *c in [JK65Records().keyEnumerator.allObjects copy]){UIView *ow=[JK65Owners() objectForKey:c];if(ow==table||!ow)JK65RestoreCell(c,NO);}
}
static void JK65RestoreAll(void){for(UIView *c in [JK65Records().keyEnumerator.allObjects copy])JK65RestoreCell(c,NO);}
static void JK65SetStyle(NSInteger style){
    if(style<=0||style>5)[NSUserDefaults.standardUserDefaults removeObjectForKey:JK65Key];
    else [NSUserDefaults.standardUserDefaults setInteger:style forKey:JK65Key];
    JK65RestoreAll();
}
static BOOL JK65IsActiveFeed(UIScrollView *t){
    UIWindow *w=t.window;if(!w||t.hidden||t.alpha<.01)return NO;
    CGRect r=[t convertRect:t.bounds toView:w];CGRect ix=CGRectIntersection(r,w.bounds);
    if(CGRectIsNull(ix)||ix.size.width<w.bounds.size.width*.5||ix.size.height<w.bounds.size.height*.5)return NO;
    return t.bounds.size.height>200&&t.bounds.size.width>200;
}
static const void *JK65LastOffsetKey=&JK65LastOffsetKey;
static void JK65Before(UIScrollView *t){JK65RestoreTable(t);}
static void JK65After(UIScrollView *t){
    NSInteger style=JK65Effective(JK65Style(),UIAccessibilityIsReduceMotionEnabled());
    if(style==0||!JK65IsActiveFeed(t))return;
    NSArray<UITableViewCell *> *cells=[(UITableView *)t visibleCells];
    if(cells.count==0||cells.count>8)return;
    double H=t.bounds.size.height,vc=t.contentOffset.y+H/2;
    NSNumber *last=objc_getAssociatedObject(t,JK65LastOffsetKey);
    NSNumber *dirN=objc_getAssociatedObject(t,&JK65LastOffsetKey+1);
    double off=t.contentOffset.y;BOOL forward=dirN?dirN.boolValue:YES;
    if(last&&fabs(off-last.doubleValue)>.01)forward=off>last.doubleValue;
    objc_setAssociatedObject(t,JK65LastOffsetKey,@(off),OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    objc_setAssociatedObject(t,&JK65LastOffsetKey+1,@(forward),OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    for(UITableViewCell *cell in cells){
        if(fabs(cell.bounds.size.height-H)>2)continue;
        double p=(cell.center.y-vc)/H;
        if(!isfinite(p)||fabs(p)>=1||fabs(p)<.0005)continue;
        CATransform3D base=cell.layer.transform;float op=cell.layer.opacity;double z=cell.layer.zPosition;
        JK65Out o=JK65Params(style,p,forward,H);
        CATransform3D m=JK65Matrix(style,o,p,H,base);
        float nop=op*(float)o.alpha;double nz=z+o.z;
        [CATransaction begin];[CATransaction setDisableActions:YES];
        cell.layer.transform=m;cell.layer.opacity=nop;cell.layer.zPosition=nz;
        [CATransaction commit];
        [JK65Records() setObject:@[[NSValue valueWithCATransform3D:base],[NSValue valueWithCATransform3D:cell.layer.transform],@(op),@(cell.layer.opacity),@(z),@(cell.layer.zPosition)] forKey:cell];
        [JK65Owners() setObject:t forKey:cell];
    }
}
static void JK65Install(void){
    static dispatch_once_t o;dispatch_once(&o,^{
        [NSNotificationCenter.defaultCenter addObserverForName:UIAccessibilityReduceMotionStatusDidChangeNotification object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *n){JK65RestoreAll();}];
        [NSNotificationCenter.defaultCenter addObserverForName:UIDeviceOrientationDidChangeNotification object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *n){JK65RestoreAll();}];
    });
}

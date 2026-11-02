#include "MascotPacks.inc"
// Original Joker mascot. No Shijima implementation, artwork or runtime dependency.
@interface TTJokerPetView : UIImageView
@property(nonatomic,strong) UIImageView *art;
@property(nonatomic) CGPoint poseAnchor;
@end
@implementation TTJokerPetView
- (void)setImage:(UIImage *)image {
    if(!self.art){self.art=[UIImageView new];self.art.contentMode=UIViewContentModeScaleAspectFit;self.art.userInteractionEnabled=NO;[self addSubview:self.art];}
    self.art.image=image;[self setNeedsLayout];
}
- (UIImage *)image {return self.art.image;}
- (void)setPoseAnchor:(CGPoint)poseAnchor {_poseAnchor=poseAnchor;[self setNeedsLayout];}
- (void)layoutSubviews {
    [super layoutSubviews];
    CGFloat size=self.bounds.size.width;
    self.art.frame=CGRectOffset(self.bounds,(64-self.poseAnchor.x)/128*size,(128-self.poseAnchor.y)/128*size);
}
- (BOOL)pointInside:(CGPoint)p withEvent:(UIEvent *)event {
    return !self.hidden && self.alpha>.01 && CGRectContainsPoint(CGRectInset(self.bounds,-5,-5),p);
}
@end
@interface TTJokerPet : NSObject
@property(nonatomic,strong) TTJokerPetView *sprite;
@property(nonatomic,strong) CADisplayLink *link;
@property(nonatomic,weak) UIView *host;
@property(nonatomic,weak) UITabBar *bar;
@property(nonatomic) CGPoint velocity;
@property(nonatomic) BOOL dragging,covered,uiHidden;
@property(nonatomic) CFTimeInterval last,time,idle;
@property(nonatomic) NSInteger direction,kind;
@property(nonatomic,strong) NSArray<UIImage *> *frames;
@property(nonatomic,strong) NSDictionary<NSNumber *,UIImage *> *packFrames;
@property(nonatomic,copy) NSString *packID;
@property(nonatomic,copy) NSString *state;
@property(nonatomic) CGFloat climbUntil;
@property(nonatomic,strong) NSDictionary *packActions;
@property(nonatomic,copy) NSString *behavior;
@property(nonatomic) CGFloat remaining,actionAge,dragVelocity,swing,bounceBase,nextScan;
@property(nonatomic) uint32_t randomSeed;
@property(nonatomic) NSInteger wallSide;
@property(nonatomic) CGRect attachedRect;
@property(nonatomic,strong) NSHashTable<UIView *> *weakSurfaces;
- (void)advance:(CGFloat)dt;
- (void)begin:(NSString *)action duration:(CGFloat)duration;
- (void)scanSurfaces;
- (NSArray<NSValue *> *)surfaces;
- (void)chooseFloor;

- (void)updateSprite:(NSString *)state;
+ (instancetype)shared;
- (void)attach:(UIViewController *)controller;
- (void)refresh;
@end
@implementation TTJokerPet
+ (instancetype)shared {static TTJokerPet *p;static dispatch_once_t once;dispatch_once(&once,^{p=[self new];});return p;}
- (instancetype)init {
    if((self=[super init])) {
        _direction=1;
        _sprite=[[TTJokerPetView alloc] initWithFrame:CGRectMake(40,100,44,44)];
        _sprite.userInteractionEnabled=YES;_sprite.contentMode=UIViewContentModeScaleAspectFit;
        _sprite.accessibilityLabel=@"Joker mascot";_sprite.isAccessibilityElement=YES;
        [_sprite addGestureRecognizer:[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(drag:)]];
        NSNotificationCenter *n=NSNotificationCenter.defaultCenter;
        [n addObserver:self selector:@selector(refresh) name:UIApplicationDidBecomeActiveNotification object:nil];
        [n addObserver:self selector:@selector(pause) name:UIApplicationWillResignActiveNotification object:nil];
    }return self;
}
- (void)pause {self.link.paused=YES;self.last=0;self.sprite.hidden=YES;}
- (NSArray *)drawFrames:(NSInteger)kind {
    NSMutableArray *a=[NSMutableArray new];
    for(NSInteger frame=0;frame<4;frame++) {
        UIGraphicsImageRenderer *r=[[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(48,48)];
        UIImage *im=[r imageWithActions:^(UIGraphicsImageRendererContext *ctx){
            CGContextRef c=ctx.CGContext;
            UIColor *body=kind==0?[UIColor colorWithRed:.98 green:.68 blue:.25 alpha:1]:[UIColor colorWithRed:.58 green:.39 blue:.94 alpha:1];
            [body setFill];[[UIBezierPath bezierPathWithRoundedRect:CGRectMake(12,21,26,21) cornerRadius:10] fill];
            CGFloat step=(frame==1?3:frame==3?-3:0);
            [[UIBezierPath bezierPathWithRoundedRect:CGRectMake(13+step,38,9,5) cornerRadius:2] fill];
            [[UIBezierPath bezierPathWithRoundedRect:CGRectMake(29-step,38,9,5) cornerRadius:2] fill];
            UIBezierPath *ears=[UIBezierPath bezierPath];[ears moveToPoint:CGPointMake(12,24)];
            [ears addLineToPoint:CGPointMake(12,6)];[ears addLineToPoint:CGPointMake(23,17)];
            [ears addLineToPoint:CGPointMake(34,6)];[ears addLineToPoint:CGPointMake(39,25)];[ears closePath];[ears fill];
            if(kind==1){[UIColor.systemMintColor setFill];[[UIBezierPath bezierPathWithOvalInRect:CGRectMake(8,4,8,8)] fill];[[UIBezierPath bezierPathWithOvalInRect:CGRectMake(32,4,8,8)] fill];}
            [body setFill];[[UIBezierPath bezierPathWithRoundedRect:CGRectMake(10,16,30,20) cornerRadius:10] fill];
            [UIColor.blackColor setFill];
            CGFloat eye=frame==0?2:4;
            [[UIBezierPath bezierPathWithRoundedRect:CGRectMake(17,23,3,eye) cornerRadius:1.5] fill];
            [[UIBezierPath bezierPathWithRoundedRect:CGRectMake(30,23,3,eye) cornerRadius:1.5] fill];
            CGContextSetStrokeColorWithColor(c,UIColor.blackColor.CGColor);CGContextSetLineWidth(c,1.3);
            CGContextMoveToPoint(c,22,30);CGContextAddLineToPoint(c,25,32);CGContextAddLineToPoint(c,28,30);CGContextStrokePath(c);
            [body setStroke];UIBezierPath *tail=[UIBezierPath bezierPath];tail.lineWidth=4;tail.lineCapStyle=kCGLineCapRound;
            [tail moveToPoint:CGPointMake(36,36)];[tail addCurveToPoint:CGPointMake(43,24+step) controlPoint1:CGPointMake(47,40) controlPoint2:CGPointMake(45,29)];[tail stroke];
        }];[a addObject:im];
    }return a;
}
- (void)attach:(UIViewController *)controller {
    if(!controller.isViewLoaded || !controller.view.window)return;
    UIView *host=controller.view.window;
    if(self.host!=host){self.host=host;[host addSubview:self.sprite];self.sprite.center=CGPointMake(70,140);}
    // Known tab controller or ancestor only, no full-window search per animation frame.
    UITabBarController *tabs=controller.tabBarController;
    self.bar=tabs.tabBar;
    [self refresh];
}
- (CGFloat)floorY {
    CGFloat floor=CGRectGetHeight(self.host.bounds)-self.host.safeAreaInsets.bottom;
    if(self.bar.window==self.host && !self.bar.hidden && self.bar.alpha>.01){
        CGRect r=[self.bar convertRect:self.bar.bounds toView:self.host];if(r.origin.y>80)floor=MIN(floor,r.origin.y);
    }return floor-self.sprite.bounds.size.height/2;
}
- (void)refresh {
    BOOL show=PB(@"tt.pet.enabled",NO) && self.host && !self.host.hidden && !self.covered && !self.uiHidden &&
        UIApplication.sharedApplication.applicationState==UIApplicationStateActive;
    self.sprite.hidden=!show;
    CGFloat size=MAX(28,MIN(76, [D() objectForKey:@"tt.pet.size"]?[D() doubleForKey:@"tt.pet.size"]:44));
    self.sprite.bounds=CGRectMake(0,0,size,size);
    NSInteger kind=[D() integerForKey:@"tt.pet.kind"]==1?1:0;
    if(!self.frames || self.kind!=kind){self.kind=kind;self.frames=[self drawFrames:kind];self.sprite.image=self.frames[1];}
    NSString *pack=[D() stringForKey:@"tt.pet.pack"] ?: @"";
    if(![self.packID isEqualToString:pack]){self.packID=pack;self.packFrames=TTPackLoad(pack);self.packActions=TTPackActions(pack);}
    [self updateSprite:@"stand"];
    if(show && !UIAccessibilityIsReduceMotionEnabled()){
        if(!self.link){self.link=[CADisplayLink displayLinkWithTarget:self selector:@selector(step:)];self.link.preferredFramesPerSecond=30;[self.link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];}
        self.link.paused=NO;
    } else {self.link.paused=YES;self.last=0;}
    if(show){[self.host bringSubviewToFront:self.sprite];CGPoint p=self.sprite.center;p.y=MIN(p.y,[self floorY]);self.sprite.center=p;}
}
- (void)step:(CADisplayLink *)link {
    if(!self.host || self.host.hidden || self.sprite.hidden || self.covered || self.uiHidden ||
       UIApplication.sharedApplication.applicationState!=UIApplicationStateActive){self.link.paused=YES;self.last=0;return;}
    CGFloat dt=self.last?MIN(.04,MAX(0,link.timestamp-self.last)):0;self.last=link.timestamp;[self advance:dt];
}
- (uint32_t)random:(uint32_t)upper {
    if(!self.randomSeed)self.randomSeed=arc4random() ?: 1;
    self.randomSeed=self.randomSeed*1664525u+1013904223u;
    return upper?self.randomSeed%upper:0;
}
- (void)begin:(NSString *)action duration:(CGFloat)duration {
    self.behavior=action;self.remaining=duration;self.actionAge=0;self.velocity=CGPointZero;
    if([action isEqual:@"jump"])self.velocity=CGPointMake(self.direction*95,-520);
}
- (CGFloat)topY {return self.host.safeAreaInsets.top+self.sprite.bounds.size.height/2;}
- (void)scanSurfaces {
    // Bounded, low-frequency discovery. Weak references never keep feed cells alive.
    self.nextScan=self.time+.6;
    if(!self.host)return;
    NSHashTable *found=[NSHashTable weakObjectsHashTable];
    NSMutableArray *queue=[NSMutableArray arrayWithObject:self.host];
    NSUInteger i=0;
    while(i<queue.count && i<1400){
        UIView *view=queue[i++];
        if(view.hidden || view.alpha<.01 || view==self.sprite || [view isKindOfClass:TTJokerPetView.class])continue;
        NSString *cls=NSStringFromClass(view.class).lowercaseString;
        NSString *label=(view.accessibilityIdentifier ?: view.accessibilityLabel ?: @"").lowercaseString;
        CGRect r=[view convertRect:view.bounds toView:self.host];
        BOOL identified=[cls containsString:@"feedvideobutton"] || ([cls containsString:@"feed"] && [cls containsString:@"button"]);
        if([view isKindOfClass:UIControl.class]){
            for(NSString *key in @[@"like",@"comment",@"share",@"bookmark",@"favorite",@"collect",@"digg"])
                if([label containsString:key])identified=YES;
        }
        if(identified && r.origin.x>self.host.bounds.size.width*.58 && r.size.width>=20 && r.size.width<100 &&
            r.size.height>=20 && r.size.height<130 && CGRectIntersectsRect(r,self.host.bounds)){
            [found addObject:view];
        }else if(queue.count<1400)[queue addObjectsFromArray:view.subviews];
    }
    self.weakSurfaces=found;
}
- (NSArray<NSValue *> *)surfaces {
    NSMutableArray *rects=[NSMutableArray new];
    for(UIView *view in self.weakSurfaces){
        if(view.window!=self.host || view.hidden || view.alpha<.01)continue;
        BOOL visible=YES;for(UIView *p=view.superview;p && p!=self.host;p=p.superview)if(p.hidden || p.alpha<.01){visible=NO;break;}
        if(!visible)continue;
        CGRect r=[view convertRect:view.bounds toView:self.host];
        if(CGRectIntersectsRect(r,self.host.bounds))[rects addObject:[NSValue valueWithCGRect:r]];
    }return rects;
}
- (void)chooseFloor {
    // Native context-aware weighted behaviours, based on Shimeji-ee's stand/sit 200,
    // walk/run 100 and crawl 10 ratios, with mobile-friendly shorter durations.
    NSArray *names=@[@"stand",@"walk",@"run",@"dash",@"sit",@"lookup",@"dangle",@"lie",@"crawl",@"jump",@"spin",@"sprawl",@"trip"];
    NSArray *weights=@[@100,@200,@100,@35,@200,@60,@70,@55,@10,@60,@35,@30,@20];
    NSUInteger total=0;for(NSNumber *n in weights)total+=n.unsignedIntegerValue;
    NSUInteger pick=[self random:(uint32_t)total],index=0;
    for(;index+1<weights.count;index++){if(pick<weights[index].unsignedIntegerValue)break;pick-=weights[index].unsignedIntegerValue;}
    if([self random:4]==0)self.direction=-self.direction;
    NSString *a=names[index];
    [self begin:a duration:[a isEqual:@"trip"]?.65:1.2+[self random:340]/100.];
}
- (void)updateSprite:(NSString *)state {
    self.state=state;
    if(self.packFrames) {
        NSArray *poses=self.packActions[state] ?: TTDefaultPetActions()[state] ?: TTDefaultPetActions()[@"stand"];
        NSArray *pose=poses.firstObject;
        if([state isEqual:@"drag"]){
            // Native velocity bands replace all Pinched expression conditions.
            NSInteger band=self.dragVelocity>650?0:self.dragVelocity>180?1:self.dragVelocity>=-180?2:self.dragVelocity>=-650?3:4;
            pose=poses[MIN(band,(NSInteger)poses.count-1)];
        }else{
            CGFloat length=0;for(NSArray *p in poses)length+=[p[1] doubleValue]/40.;
            CGFloat phase=fmod(MAX(0,self.actionAge),MAX(.025,length));
            for(NSArray *p in poses){pose=p;phase-=[p[1] doubleValue]/40.;if(phase<0)break;}
        }
        NSNumber *n=pose[0];BOOL available=self.packFrames[n]!=nil;
        self.sprite.image=self.packFrames[n] ?: self.packFrames[@1];
        self.sprite.poseAnchor=available?CGPointMake([pose[2] doubleValue],[pose[3] doubleValue]):CGPointMake(64,128);
    } else {
        BOOL moving=[@[@"walk",@"run",@"dash",@"crawl",@"climb",@"ceiling"] containsObject:state];
        CGFloat speed=[state isEqual:@"run"] || [state isEqual:@"dash"]?14:6;
        self.sprite.image=self.frames[moving?((NSInteger)(self.time*speed)%3)+1:((NSInteger)(self.time*2)%9==0?0:1)];
        self.sprite.poseAnchor=CGPointMake(64,128);
    }
    CGFloat face=self.packFrames?-self.direction:self.direction;
    CGAffineTransform transform=CGAffineTransformMakeScale(face,1);
    if([state isEqual:@"drag"])transform=CGAffineTransformRotate(transform,self.swing);
    self.sprite.transform=transform;
}
- (void)advance:(CGFloat)dt {
    dt=MAX(0,MIN(.04,dt));self.time+=dt;self.actionAge+=dt;
    if(self.time>=self.nextScan)[self scanSurfaces];
    if(self.dragging){self.dragVelocity*=pow(.16,dt);self.swing+=(MAX(-.65,MIN(.65,-self.dragVelocity/1900))-self.swing)*MIN(1,dt*9);[self updateSprite:@"drag"];return;}
    CGPoint p=self.sprite.center,old=p;CGFloat half=self.sprite.bounds.size.width/2,floor=[self floorY],top=[self topY],width=self.host.bounds.size.width;
    NSArray *rects=[self surfaces];
    if(!self.behavior.length)[self begin:@"fall" duration:10];
    self.remaining-=dt;
    if([self.behavior isEqual:@"bounce"]){
        p.y=self.bounceBase-sin(MIN(1,self.actionAge/.24)*M_PI)*7;
        if(self.remaining<=0){p.y=self.bounceBase;[self begin:@"stand" duration:.5];}
        self.sprite.center=p;[self updateSprite:self.behavior];return;
    }
    BOOL wall=[@[@"climb",@"grab",@"wallsit"] containsObject:self.behavior];
    BOOL ceiling=[@[@"ceiling",@"hang"] containsObject:self.behavior];
    if(wall || ceiling){
        // A moving/removed action button invalidates its attachment immediately.
        BOOL attached=CGRectIsEmpty(self.attachedRect);
        for(NSValue *value in rects)if(CGRectEqualToRect(value.CGRectValue,self.attachedRect))attached=YES;
        if(!attached){[self begin:@"fall" duration:10];wall=NO;ceiling=NO;}
    }
    if(wall){
        CGFloat wallTop=CGRectIsEmpty(self.attachedRect)?top:CGRectGetMinY(self.attachedRect)-half;
        CGFloat wallBottom=CGRectIsEmpty(self.attachedRect)?floor:CGRectGetMaxY(self.attachedRect)+half;
        p.x=CGRectIsEmpty(self.attachedRect)?(self.wallSide<0?half:width-half):(self.wallSide<0?CGRectGetMinX(self.attachedRect)-half:CGRectGetMaxX(self.attachedRect)+half);
        if([self.behavior isEqual:@"climb"])p.y-=52*dt;
        if(p.y<=wallTop){
            p.y=wallTop;
            if(CGRectIsEmpty(self.attachedRect)){[self begin:@"ceiling" duration:4+[self random:400]/100.];self.direction=self.wallSide<0?1:-1;}
            else {p.x+=self.wallSide<0?half:-half;[self begin:@"dangle" duration:2];self.attachedRect=CGRectZero;}
        }else if(p.y>wallBottom+2){[self begin:@"fall" duration:10];}
        else if(self.remaining<=0){
            NSUInteger choice=[self random:5];
            if(choice==0){[self begin:@"fall" duration:10];self.velocity=CGPointMake(-self.wallSide*85,30);p.x-=self.wallSide*3;}
            else [self begin:choice==1?@"wallsit":choice==2?@"grab":@"climb" duration:choice>2?12:2];
        }
        self.sprite.center=p;[self updateSprite:self.behavior];return;
    }
    if(ceiling){
        CGFloat underside=CGRectIsEmpty(self.attachedRect)?top:CGRectGetMaxY(self.attachedRect)+half;
        p.y=underside;
        if([self.behavior isEqual:@"ceiling"])p.x+=self.direction*35*dt;
        CGFloat left=CGRectIsEmpty(self.attachedRect)?half:CGRectGetMinX(self.attachedRect);
        CGFloat right=CGRectIsEmpty(self.attachedRect)?width-half:CGRectGetMaxX(self.attachedRect);
        if(p.x<=left || p.x>=right){
            p.x=MAX(left,MIN(right,p.x));self.wallSide=p.x<=left?-1:1;
            if(CGRectIsEmpty(self.attachedRect)){self.direction=-self.direction;[self begin:@"hang" duration:1];}
            else [self begin:@"climb" duration:8];
        }else if(self.remaining<=0){
            if([self random:4]==0){[self begin:@"fall" duration:10];p.y+=3;}
            else [self begin:[self random:3]==0?@"hang":@"ceiling" duration:2+[self random:400]/100.];
        }self.sprite.center=p;[self updateSprite:self.behavior];return;
    }
    CGFloat support=floor;
    for(NSValue *value in rects){
        CGRect r=value.CGRectValue;CGFloat y=r.origin.y-half;
        if(p.x>=r.origin.x-2 && p.x<=CGRectGetMaxX(r)+2 && fabs(p.y-y)<3)support=MIN(support,y);
    }
    BOOL grounded=fabs(p.y-support)<1 && fabs(self.velocity.y)<1;
    BOOL airborne=[@[@"jump",@"fall",@"thrown"] containsObject:self.behavior];
    if(grounded && !airborne){
        if(self.remaining<=0)[self chooseFloor];
        CGFloat speed=[self.behavior isEqual:@"walk"]?42:[self.behavior isEqual:@"run"]?95:[self.behavior isEqual:@"dash"]?175:[self.behavior isEqual:@"crawl"]?13:[self.behavior isEqual:@"trip"]?40:0;
        p.x+=self.direction*speed*dt;
        if([self.behavior isEqual:@"jump"]){grounded=NO;airborne=YES;}
        else {
            p.y=support;
            BOOL supported=support==floor;
            for(NSValue *value in rects){CGRect r=value.CGRectValue;if(fabs(r.origin.y-half-support)<1 && p.x>=r.origin.x && p.x<=CGRectGetMaxX(r))supported=YES;}
            if(!supported){[self begin:@"fall" duration:10];grounded=NO;}
        }
    }
    if(!grounded || airborne){
        if(!airborne && ![@[@"jump",@"thrown"] containsObject:self.behavior])[self begin:@"fall" duration:10];
        self.velocity=CGPointMake(self.velocity.x*pow(.72,dt),self.velocity.y+950*dt);
        p.x+=self.velocity.x*dt;p.y+=self.velocity.y*dt;
        CGFloat landing=floor;BOOL hit=p.y>=floor;
        for(NSValue *value in rects){
            CGRect r=value.CGRectValue;CGFloat y=r.origin.y-half;
            if(self.velocity.y>=0 && old.y<=y+.5 && p.y>=y && p.x>=r.origin.x && p.x<=CGRectGetMaxX(r) && y<landing){landing=y;hit=YES;}
            // Swept underside collision when jumping into an action surface.
            if(self.velocity.y<0 && old.y-half>=CGRectGetMaxY(r) && p.y-half<=CGRectGetMaxY(r) && p.x>=r.origin.x && p.x<=CGRectGetMaxX(r)){
                self.attachedRect=r;p.y=CGRectGetMaxY(r)+half;[self begin:@"hang" duration:1.5];hit=NO;break;
            }
            if(p.y+half>r.origin.y && p.y-half<CGRectGetMaxY(r) && fabs(self.velocity.x)>40){
                CGFloat left=r.origin.x-half,right=CGRectGetMaxX(r)+half;
                if((old.x<=left && p.x>=left)||(old.x>=right && p.x<=right)){
                    self.wallSide=old.x<=left?-1:1;p.x=self.wallSide<0?left:right;self.attachedRect=r;[self begin:@"grab" duration:1];hit=NO;break;
                }
            }
        }
        if(hit && self.velocity.y>=0){
            p.y=landing;[self begin:@"bounce" duration:.24];
            // Bouncing uses the standard compressed/recovery poses and a small native hop.
            self.bounceBase=landing;
        }
        if(p.y<=top){p.y=top;self.attachedRect=CGRectZero;[self begin:@"hang" duration:1.5];}
    }
    if(p.x<=half || p.x>=width-half){
        self.wallSide=p.x<=half?-1:1;p.x=MAX(half,MIN(width-half,p.x));self.attachedRect=CGRectZero;
        self.direction=self.wallSide<0?1:-1;
        [self begin:[self random:4]==0?@"wallsit":@"climb" duration:18];
    }
    p.x=MAX(half,MIN(width-half,p.x));p.y=MAX(top,MIN(floor,p.y));
    self.sprite.center=p;[self updateSprite:self.behavior];
}
- (void)drag:(UIPanGestureRecognizer *)pan {
    CGPoint t=[pan translationInView:self.host];
    if(pan.state==UIGestureRecognizerStateBegan){
        self.dragging=YES;self.velocity=CGPointZero;self.climbUntil=0;self.attachedRect=CGRectZero;self.swing=0;self.actionAge=0;self.dragVelocity=0;
    }
    if(pan.state==UIGestureRecognizerStateChanged){
        self.dragVelocity=[pan velocityInView:self.host].x;
        CGPoint p=self.sprite.center;CGFloat h=self.sprite.bounds.size.width/2;
        p.x=MAX(h,MIN(self.host.bounds.size.width-h,p.x+t.x));
        p.y=MAX([self topY],MIN([self floorY],p.y+t.y));self.sprite.center=p;
        self.swing=MAX(-.6,MIN(.6,-self.dragVelocity/1900));[self updateSprite:@"drag"];
    }
    [pan setTranslation:CGPointZero inView:self.host];
    if(pan.state==UIGestureRecognizerStateEnded || pan.state==UIGestureRecognizerStateCancelled || pan.state==UIGestureRecognizerStateFailed){
        self.dragging=NO;CGPoint v=[pan velocityInView:self.host];
        [self begin:@"thrown" duration:10];
        self.velocity=pan.state==UIGestureRecognizerStateEnded?CGPointMake(MAX(-900,MIN(900,v.x)),MAX(-1200,MIN(1200,v.y))):CGPointZero;
        self.idle=0;self.last=0;
    }
}
@end

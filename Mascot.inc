#include "MascotPacks.inc"
// Original Joker mascot. No Shijima implementation, artwork or runtime dependency.
@interface TTJokerPetView : UIImageView
@end
@implementation TTJokerPetView
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
    if(![self.packID isEqualToString:pack]){self.packID=pack;self.packFrames=TTPackLoad(pack);}
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
- (void)updateSprite:(NSString *)state {
    self.state=state;
    if(self.packFrames) {
        // Conventional Shimeji poses. XML actions/behaviors are intentionally ignored.
        NSArray *poses=[state isEqual:@"walk"]?@[@1,@2,@1,@3]:
            [state isEqual:@"fall"]?@[@4]:
            [state isEqual:@"drag"]?@[@5,@6]:
            [state isEqual:@"sit"]?@[@11]:
            [state isEqual:@"climb"]?@[@12,@13]:@[@1];
        NSNumber *n=poses[(NSInteger)(self.time*6)%poses.count];
        self.sprite.image=self.packFrames[n] ?: self.packFrames[@1];
    } else {
        NSInteger f=[state isEqual:@"walk"]?((NSInteger)(self.time*6)%3)+1:((NSInteger)(self.time*2)%9==0?0:1);
        self.sprite.image=self.frames[f];
    }
    self.sprite.transform=CGAffineTransformMakeScale(self.direction,1);
}
- (void)advance:(CGFloat)dt {
    self.time+=dt;
    if(self.dragging){[self updateSprite:@"drag"];return;}
    CGPoint p=self.sprite.center;CGFloat half=self.sprite.bounds.size.width/2,floor=[self floorY];
    if(self.climbUntil>self.time) {
        p.y=MAX(self.host.safeAreaInsets.top+half,p.y-24*dt);
        self.sprite.center=p;self.velocity=CGPointZero;[self updateSprite:@"climb"];return;
    }
    self.velocity=CGPointMake(self.velocity.x,self.velocity.y+1100*dt);
    p.x+=self.velocity.x*dt;p.y+=self.velocity.y*dt;
    if(p.y>=floor){
        p.y=floor;
        self.velocity=CGPointMake(self.velocity.x*.9,fabs(self.velocity.y)>180?-self.velocity.y*.22:0);
        if(fabs(self.velocity.y)<1){
            self.idle+=dt;
            // Alternate walking along the bottom edge and sitting/blinking.
            if(fmod(self.idle,10)<6)p.x+=self.direction*32*dt;
        }
    }
    if(p.x<half){if(p.y<floor-5 && fabs(self.velocity.x)>100)self.climbUntil=self.time+1.2;p.x=half;self.direction=1;self.velocity=CGPointMake(fabs(self.velocity.x)*.5,self.velocity.y);}
    if(p.x>self.host.bounds.size.width-half){if(p.y<floor-5 && fabs(self.velocity.x)>100)self.climbUntil=self.time+1.2;p.x=self.host.bounds.size.width-half;self.direction=-1;self.velocity=CGPointMake(-fabs(self.velocity.x)*.5,self.velocity.y);}
    p.y=MAX(self.host.safeAreaInsets.top+half,p.y);self.sprite.center=p;
    BOOL walking=p.y>=floor-.5 && fmod(self.idle,10)<6;
    [self updateSprite:p.y<floor-.5?@"fall":walking?@"walk":@"sit"];
}
- (void)drag:(UIPanGestureRecognizer *)pan {
    CGPoint t=[pan translationInView:self.host];
    if(pan.state==UIGestureRecognizerStateBegan){self.dragging=YES;self.velocity=CGPointZero;self.climbUntil=0;[self updateSprite:@"drag"];}
    if(pan.state==UIGestureRecognizerStateChanged){
        CGPoint p=self.sprite.center;CGFloat h=self.sprite.bounds.size.width/2;
        p.x=MAX(h,MIN(self.host.bounds.size.width-h,p.x+t.x));
        p.y=MAX(self.host.safeAreaInsets.top+h,MIN([self floorY],p.y+t.y));self.sprite.center=p;
    }
    [pan setTranslation:CGPointZero inView:self.host];
    if(pan.state==UIGestureRecognizerStateEnded || pan.state==UIGestureRecognizerStateCancelled || pan.state==UIGestureRecognizerStateFailed){
        self.dragging=NO;CGPoint v=[pan velocityInView:self.host];self.velocity=pan.state==UIGestureRecognizerStateEnded?CGPointMake(MAX(-900,MIN(900,v.x)),MAX(-1200,MIN(1200,v.y))):CGPointZero;
        self.idle=0;self.last=0;
    }
}
@end

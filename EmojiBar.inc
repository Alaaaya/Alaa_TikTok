// Bounded, local-only diagnostics contain class names/geometry, never feed text or user data.
static CFTimeInterval TT48LastPress=0;
static NSMapTable<UIView *,NSNumber *> *TT48Births(void) {
    static NSMapTable *map;static dispatch_once_t once;
    dispatch_once(&once,^{map=[NSMapTable weakToStrongObjectsMapTable];});return map;
}
static NSArray<UIWindow *> *TT48Windows(void) {
    NSMutableArray *windows=[NSMutableArray new];
    for(UIScene *scene in UIApplication.sharedApplication.connectedScenes) {
        if(![scene isKindOfClass:UIWindowScene.class] || scene.activationState!=UISceneActivationStateForegroundActive)continue;
        for(UIWindow *window in ((UIWindowScene *)scene).windows)
            if(!window.hidden && window.alpha>.05 && ![NSStringFromClass(window.class).lowercaseString containsString:@"keyboard"])
                [windows addObject:window];
    }
    return windows;
}
static NSArray<UIWindow *> *TT50DiagnosticWindows(void) {
    NSMutableOrderedSet *windows=[NSMutableOrderedSet new];
    // Enumerate at capture time, not at touch time: popups may create new windows.
    for(UIScene *scene in UIApplication.sharedApplication.connectedScenes)
        if([scene isKindOfClass:UIWindowScene.class])[windows addObjectsFromArray:((UIWindowScene *)scene).windows];
    [windows addObjectsFromArray:UIApplication.sharedApplication.windows];
    return windows.array;
}
static void TT48Debug(NSString *event) {
    if(!PB(@"tt46.emojiBar",NO))return;
    NSMutableString *log=[NSMutableString stringWithFormat:@"Joker50 %@ %.3f\n",event,CACurrentMediaTime()];
    for(UIWindow *window in TT50DiagnosticWindows()) {
        NSUInteger total=0;
        [log appendFormat:@"window %@ level %.1f hidden %d alpha %.2f\n",NSStringFromClass(window.class),window.windowLevel,window.hidden,window.alpha];
        NSMutableArray *queue=[NSMutableArray arrayWithObject:window];NSUInteger i=0;
        while(i<queue.count && total<900) {
            UIView *v=queue[i++];total++;
            CGRect r=[v convertRect:v.bounds toView:window];
            [log appendFormat:@"%@ x%.0f y%.0f w%.0f h%.0f sub%lu radius%.0f\n",NSStringFromClass(v.class),r.origin.x,r.origin.y,r.size.width,r.size.height,(unsigned long)v.subviews.count,v.layer.cornerRadius];
            if(queue.count<1200)[queue addObjectsFromArray:v.subviews];
        }
    }
    NSData *data=[log dataUsingEncoding:NSUTF8StringEncoding];
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_UTILITY,0),^{
        @synchronized(NSFileManager.defaultManager) {
            NSString *folder=NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject;
            NSString *path=[folder stringByAppendingPathComponent:@"joker_debug.txt"];
            unsigned long long size=[[NSFileManager.defaultManager attributesOfItemAtPath:path error:nil] fileSize];
            if(size>256*1024)[NSFileManager.defaultManager removeItemAtPath:path error:nil];
            if(![NSFileManager.defaultManager fileExistsAtPath:path])[NSData.data writeToFile:path atomically:YES];
            NSFileHandle *file=[NSFileHandle fileHandleForWritingAtPath:path];
            @try {[file seekToEndOfFile];[file writeData:data];[file closeFile];} @catch(NSException *exception) {}
        }
    });
}
// Runtime hierarchy recognition, not guessed private class hooks. Unsupported bars are untouched.
static BOOL TT46EmojiText(NSString *text) {
    if(!text.length || text.length>16)return NO;
    __block NSUInteger count=0;__block BOOL valid=YES;
    [text enumerateSubstringsInRange:NSMakeRange(0,text.length) options:NSStringEnumerationByComposedCharacterSequences
                         usingBlock:^(NSString *piece,NSRange a,NSRange b,BOOL *stop){
        if([piece stringByTrimmingCharactersInSet:NSCharacterSet.whitespaceAndNewlineCharacterSet].length==0)return;
        NSData *data=[piece dataUsingEncoding:NSUTF32LittleEndianStringEncoding];
        BOOL emoji=NO;
        for(NSUInteger i=0;i+4<=data.length;i+=4){
            uint32_t scalar=0;[data getBytes:&scalar range:NSMakeRange(i,4)];
            if((scalar>=0x1F300 && scalar<=0x1FAFF)||(scalar>=0x2600 && scalar<=0x27BF))emoji=YES;
        }
        count++;if(!emoji)valid=NO;
    }];
    return valid && count==1;
}
static void TT48Frame(UIView *v,CGRect r) {
    v.transform=CGAffineTransformIdentity;v.layer.bounds=(CGRect){CGPointZero,r.size};
    v.layer.position=CGPointMake(r.origin.x+r.size.width*v.layer.anchorPoint.x,r.origin.y+r.size.height*v.layer.anchorPoint.y);
}
@interface TT46PillExtension : UIView
@end
@implementation TT46PillExtension
- (BOOL)pointInside:(CGPoint)p withEvent:(UIEvent *)event {
    for(UIView *button in self.subviews)if(CGRectContainsPoint(button.frame,p))return YES;
    return NO;
}
@end
@interface TT46EmojiBar : NSObject
@property(nonatomic,weak) UIView *nativeBar;
@property(nonatomic,strong) UIView *extension;
@property(nonatomic,strong) UIButton *eye,*download;
@property(nonatomic,copy) void (^action)(BOOL eye);
@property(nonatomic,copy) NSString *detectedClass;
- (void)scan:(UIWindow *)window enabled:(BOOL)enabled;
- (void)detach;
- (void)scanWindows:(NSArray<UIWindow *> *)windows enabled:(BOOL)enabled;
@end
@implementation TT46EmojiBar
- (void)detach {[self.extension removeFromSuperview];self.extension=nil;self.nativeBar=nil;self.eye=nil;self.download=nil;}
- (NSArray<UIView *> *)emojiLeaves:(UIView *)root {
    NSMutableArray *items=[NSMutableArray new],*queue=[NSMutableArray arrayWithObject:root];
    NSUInteger i=0;
    while(i<queue.count && i<90){
        UIView *v=queue[i++];
        if(v.hidden || v.alpha<.05 || v==self.extension)continue;
        NSString *text=nil;
        if([v isKindOfClass:UILabel.class])text=((UILabel *)v).text;
        if([v isKindOfClass:UIButton.class])text=[((UIButton *)v) titleForState:UIControlStateNormal];
        if(TT46EmojiText(text)){[items addObject:v];continue;}
        [queue addObjectsFromArray:v.subviews];
    }
    return items;
}
- (BOOL)geometry:(UIView *)bar window:(UIWindow *)window rect:(CGRect *)out diameter:(CGFloat *)diameter {
    if(bar==window || bar.window!=window || bar.hidden || bar.alpha<.05)return NO;
    CGRect r=[bar convertRect:bar.bounds toView:window];
    if(r.size.height<32 || r.size.height>96 || r.size.width<130 || r.size.width>MIN(460,window.bounds.size.width-8) ||
       r.size.width/r.size.height<2.0 || !CGRectContainsRect(CGRectInset(window.bounds,-1,-1),r))return NO;
    // Both Arabic left-side and LTR right-side action columns are valid.
    if(r.origin.y<window.safeAreaInsets.top+12 || CGRectGetMaxY(r)>window.bounds.size.height-window.safeAreaInsets.bottom-40)return NO;
    for(UIView *v=bar;v;v=v.superview){
        if(v.hidden || v.alpha<.05)return NO;
        NSString *name=NSStringFromClass(v.class).lowercaseString;
        if([name containsString:@"keyboard"] || [name containsString:@"commentpanel"] ||
           [name containsString:@"textinput"] || [v isKindOfClass:UITextField.class] ||
           [v isKindOfClass:UITextView.class])return NO;
    }
    NSArray *leaves=[self emojiLeaves:bar];
    if(leaves.count<3 || leaves.count>6) {
        // Image-backed reactions require a newly appearing capsule during a long press.
        BOOL recent=bar==self.nativeBar || (TT48LastPress>0 && CACurrentMediaTime()-TT48LastPress<4);
        BOOL born=NO;
        for(UIView *v=bar;v && v!=window;v=v.superview) {
            NSNumber *birth=[TT48Births() objectForKey:v];
            if(birth && birth.doubleValue>=TT48LastPress-.3){born=YES;break;}
        }
        if(!recent || !born)return NO;
        BOOL capsule=bar.layer.cornerRadius>=r.size.height*.25;
        for(UIView *v in bar.subviews)if([v isKindOfClass:UIVisualEffectView.class] || v.layer.cornerRadius>=r.size.height*.25)capsule=YES;
        if(!capsule)return NO;
        NSMutableArray *items=[NSMutableArray new],*queue=[NSMutableArray arrayWithObject:bar];NSUInteger index=0;
        while(index<queue.count && index<140) {
            UIView *v=queue[index++];if(v.hidden || v.alpha<.05 || v==self.extension)continue;
            CGRect f=[v convertRect:v.bounds toView:window];
            BOOL image=([v isKindOfClass:UIImageView.class] && ((UIImageView *)v).image!=nil);
            BOOL button=([v isKindOfClass:UIButton.class] && [((UIButton *)v) imageForState:UIControlStateNormal]!=nil);
            if((image || button) && f.size.width>=16 && f.size.width<=76 &&
               f.size.height>=16 && f.size.height<=76 && fabs(CGRectGetMidY(f)-CGRectGetMidY(r))<r.size.height*.28) {
                [items addObject:v];continue;
            }
            [queue addObjectsFromArray:v.subviews];
        }
        leaves=items;if(leaves.count<3 || leaves.count>6)return NO;
    }
    NSMutableArray<NSValue *> *frames=[NSMutableArray new];
    for(UIView *v in leaves){
        CGRect f=[v convertRect:v.bounds toView:window];
        if(f.size.width<12 || f.size.width>72 || fabs(CGRectGetMidY(f)-CGRectGetMidY(r))>r.size.height*.22)return NO;
        [frames addObject:[NSValue valueWithCGRect:f]];
    }
    [frames sortUsingComparator:^NSComparisonResult(NSValue *a,NSValue *b){
        return CGRectGetMidX(a.CGRectValue)<CGRectGetMidX(b.CGRectValue)?NSOrderedAscending:NSOrderedDescending;
    }];
    CGFloat spacing=(CGRectGetMidX(frames.lastObject.CGRectValue)-CGRectGetMidX(frames[0].CGRectValue))/(frames.count-1);
    if(spacing<25 || spacing>82)return NO;
    for(NSUInteger i=1;i<frames.count;i++)if(fabs(CGRectGetMidX(frames[i].CGRectValue)-CGRectGetMidX(frames[i-1].CGRectValue)-spacing)>8)return NO;
    // No content is squeezed or moved. Without room at physical left we safely do nothing.
    CGFloat d=MIN(r.size.height,spacing);
    BOOL left=r.origin.x-2*d>=window.safeAreaInsets.left+4;
    BOOL right=CGRectGetMaxX(r)+2*d<=window.bounds.size.width-window.safeAreaInsets.right-4;
    if(!left && !right)return NO;
    *out=r;*diameter=d;return YES;
}
- (void)pressed:(UIButton *)sender {if(self.action)self.action(sender==self.eye);}
- (void)scan:(UIWindow *)window enabled:(BOOL)enabled {
    if(!enabled || !window || window.hidden){[self detach];return;}
    UIView *bar=nil;CGRect rect=CGRectZero;CGFloat d=0;
    NSMutableArray *queue=[NSMutableArray arrayWithObject:window];NSUInteger i=0;
    while(i<queue.count && i<1600){
        UIView *candidate=queue[i++];
        if(candidate==self.extension || candidate.hidden || candidate.alpha<.05)continue;
        if([self geometry:candidate window:window rect:&rect diameter:&d]){bar=candidate;break;}
        if(queue.count<1600)[queue addObjectsFromArray:candidate.subviews];
    }
    if(!bar){[self detach];return;}
    if(!self.extension){
        UIView *extension=[TT46PillExtension new];extension.backgroundColor=UIColor.whiteColor;
        extension.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
        extension.layer.cornerRadius=rect.size.height/2;
        extension.layer.maskedCorners=kCALayerMinXMinYCorner|kCALayerMinXMaxYCorner;
        self.extension=extension;
        for(NSInteger i=0;i<2;i++){
            UIButton *b=[UIButton buttonWithType:UIButtonTypeSystem];
            b.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
            b.tintColor=[UIColor colorWithWhite:.14 alpha:1];b.backgroundColor=UIColor.whiteColor;
            [b setImage:[UIImage systemImageNamed:i==0?@"eye":@"arrow.down"] forState:UIControlStateNormal];
            b.accessibilityLabel=i==0?@"Hide or show feed controls":@"Download menu";
            [b addTarget:self action:@selector(pressed:) forControlEvents:UIControlEventTouchUpInside];
            [extension addSubview:b];if(i==0)self.eye=b;else self.download=b;
        }
    }
    self.nativeBar=bar;
    // The companion is an attached extension of the native pill. Never change native frames,
    // constraints, gestures or hit-testing, preserving all four emoji targets unchanged.
    BOOL left=rect.origin.x-2*d>=window.safeAreaInsets.left+4;
    CGFloat bridge=rect.size.height/2;
    CGFloat x=left?rect.origin.x-2*d:CGRectGetMaxX(rect)-bridge;
    self.extension.layer.maskedCorners=left?(kCALayerMinXMinYCorner|kCALayerMinXMaxYCorner):(kCALayerMaxXMinYCorner|kCALayerMaxXMaxYCorner);
    TT48Frame(self.extension,CGRectMake(x,rect.origin.y,2*d+bridge,rect.size.height));
    CGFloat offset=left?0:bridge;
    TT48Frame(self.eye,CGRectMake(offset,(rect.size.height-d)/2,d,d));
    TT48Frame(self.download,CGRectMake(offset+d,(rect.size.height-d)/2,d,d));
    self.eye.layer.cornerRadius=d/2;self.download.layer.cornerRadius=d/2;
    if(self.extension.superview!=window)[window addSubview:self.extension];
    // Top-level companion remains visible even when the popup is nested in the feed root.
    // The overlap bridge is passthrough; only the two new circles accept touches.
    [window bringSubviewToFront:self.extension];
    NSString *cls=NSStringFromClass(bar.class);
    if(![self.detectedClass isEqual:cls]){self.detectedClass=cls;NSLog(@"[Joker48] reaction bar hierarchy class %@",cls);}
}
- (void)scanWindows:(NSArray<UIWindow *> *)windows enabled:(BOOL)enabled {
    if(!enabled){[self detach];return;}
    // High-level popup windows first; don't detach a valid result just because another window has no bar.
    NSArray *sorted=[windows sortedArrayUsingComparator:^NSComparisonResult(UIWindow *a,UIWindow *b){
        return a.windowLevel>b.windowLevel?NSOrderedAscending:NSOrderedDescending;
    }];
    for(UIWindow *window in sorted){[self scan:window enabled:YES];if(self.nativeBar)return;}
    [self detach];
}
@end

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
    if(bar==window || bar.window!=window || bar.hidden || bar.alpha<.8 || !CGAffineTransformIsIdentity(bar.transform))return NO;
    CGRect r=[bar convertRect:bar.bounds toView:window];
    if(r.size.height<32 || r.size.height>80 || r.size.width<130 || r.size.width>320 ||
       r.size.width/r.size.height<2.5 || !CGRectContainsRect(CGRectInset(window.bounds,2,2),r))return NO;
    // Restrict to the feed's right-side comment action band, not a keyboard or comment composer.
    if(CGRectGetMaxX(r)<window.bounds.size.width*.65 || r.origin.y<window.bounds.size.height*.16 ||
       CGRectGetMaxY(r)>window.bounds.size.height*.82)return NO;
    for(UIView *v=bar;v;v=v.superview){
        if(v.hidden || v.alpha<.8)return NO;
        NSString *name=NSStringFromClass(v.class).lowercaseString;
        if([name containsString:@"keyboard"] || [name containsString:@"commentpanel"] ||
           [name containsString:@"textinput"] || [v isKindOfClass:UITextField.class] ||
           [v isKindOfClass:UITextView.class])return NO;
    }
    NSArray *leaves=[self emojiLeaves:bar];if(leaves.count!=4)return NO;
    NSMutableArray<NSValue *> *frames=[NSMutableArray new];
    for(UIView *v in leaves){
        CGRect f=[v convertRect:v.bounds toView:window];
        if(f.size.width<12 || f.size.width>72 || fabs(CGRectGetMidY(f)-CGRectGetMidY(r))>r.size.height*.22)return NO;
        [frames addObject:[NSValue valueWithCGRect:f]];
    }
    [frames sortUsingComparator:^NSComparisonResult(NSValue *a,NSValue *b){
        return CGRectGetMidX(a.CGRectValue)<CGRectGetMidX(b.CGRectValue)?NSOrderedAscending:NSOrderedDescending;
    }];
    CGFloat spacing=(CGRectGetMidX(frames[3].CGRectValue)-CGRectGetMidX(frames[0].CGRectValue))/3;
    if(spacing<25 || spacing>65)return NO;
    for(int i=1;i<4;i++)if(fabs(CGRectGetMidX(frames[i].CGRectValue)-CGRectGetMidX(frames[i-1].CGRectValue)-spacing)>8)return NO;
    // No content is squeezed or moved. Without room at physical left we safely do nothing.
    CGFloat d=MIN(r.size.height,spacing);
    if(r.origin.x-2*d<window.safeAreaInsets.left+4)return NO;
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
    self.extension.frame=CGRectMake(rect.origin.x-2*d,rect.origin.y,2*d+rect.size.height/2,rect.size.height);
    self.eye.frame=CGRectMake(0,(rect.size.height-d)/2,d,d);
    self.download.frame=CGRectMake(d,(rect.size.height-d)/2,d,d);
    self.eye.layer.cornerRadius=d/2;self.download.layer.cornerRadius=d/2;
    if(self.extension.superview!=window)[window addSubview:self.extension];
    // Top-level companion remains visible even when the popup is nested in the feed root.
    // The overlap bridge is passthrough; only the two new circles accept touches.
    [window bringSubviewToFront:self.extension];
    NSString *cls=NSStringFromClass(bar.class);
    if(![self.detectedClass isEqual:cls]){self.detectedClass=cls;NSLog(@"[Joker46] reaction bar hierarchy class %@",cls);}
}
@end

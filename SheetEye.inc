// Native video sheet discovery uses visible content, not guessed private class names.
static BOOL TT51Visible(UIView *view) {
    if(!view.window)return NO;
    for(UIView *p=view;p;p=p.superview)if(p.hidden || p.alpha<.05)return NO;
    return YES;
}
static NSArray<UIView *> *TT51Descendants(UIView *root) {
    NSMutableArray *items=[NSMutableArray arrayWithObject:root];
    for(NSUInteger i=0;i<items.count && i<1600;i++) {
        UIView *v=items[i];NSString *name=NSStringFromClass(v.class);
        if([name hasPrefix:@"TTJoker"] || [name hasPrefix:@"TT122"] || [name hasPrefix:@"TT51"])continue;
        if(items.count<1600)[items addObjectsFromArray:v.subviews];
    }
    return items;
}
static NSString *TT51Text(UIView *v) {
    NSString *s=[v isKindOfClass:UILabel.class]?((UILabel *)v).text:nil;
    if([v isKindOfClass:UIButton.class])s=[((UIButton *)v) titleForState:UIControlStateNormal];
    return [[s ?: @"" stringByTrimmingCharactersInSet:NSCharacterSet.whitespaceAndNewlineCharacterSet] lowercaseString];
}
static BOOL TT51Download(UIView *v) {
    if([@[@"تنزيل",@"download",@"herunterladen",@"télécharger",@"descargar",@"baixar",@"scarica",@"indir"] containsObject:TT51Text(v)])return YES;
    // Use only explicit icon identities, never an arbitrary down arrow or guessed class.
    NSString *identity=v.accessibilityIdentifier.lowercaseString ?: @"";
    NSString *accessible=v.accessibilityLabel.lowercaseString ?: @"";
    return [@[@"download",@"arrow.down.to.line",@"square.and.arrow.down",@"arrow.down.to.line.compact"] containsObject:identity] ||
           ([v isKindOfClass:UIImageView.class] && [@[@"download",@"تنزيل"] containsObject:accessible]);
}
static BOOL TT51SheetPeers(UIView *root) {
    BOOL uninterested=NO,report=NO;
    for(UIView *v in TT51Descendants(root)) {
        if(v.hidden || v.alpha<.05)continue;
        NSString *s=TT51Text(v);
        uninterested|=[@[@"غير مهتم",@"not interested",@"nicht interessiert",@"pas intéressé",@"no me interesa"] containsObject:s];
        report|=[@[@"إبلاغ",@"ابلاغ",@"report",@"melden",@"signaler",@"denunciar"] containsObject:s];
    }
    return uninterested && report;
}
@interface TT51EyeRow:UIButton
@property(nonatomic,strong) UILabel *caption;
@property(nonatomic,strong) UIImageView *glyph;
@property(nonatomic) CGFloat iconX,textX,textWidth;
@end
@implementation TT51EyeRow
- (void)layoutSubviews {
    [super layoutSubviews];
    CGFloat h=self.bounds.size.height;
    self.glyph.frame=CGRectMake(self.iconX,(h-24)/2,24,24);
    self.caption.frame=CGRectMake(self.textX,0,MIN(self.textWidth,self.bounds.size.width-self.textX-10),h);
}
@end
@interface TT51SheetEye:NSObject
@property(nonatomic,strong) TT51EyeRow *row;
@property(nonatomic,weak) UIView *nativeRow;
@property(nonatomic,weak) UIView *scope;
@property(nonatomic,weak) UIStackView *stack;
@property(nonatomic,weak) UIScrollView *scroll;
@property(nonatomic) UIEdgeInsets oldInset,oldIndicators;
@property(nonatomic) CGFloat addedInset;
@property(nonatomic,strong) NSLayoutConstraint *stackHeight;
@property(nonatomic) CGFloat originalHeight;
@property(nonatomic) CGRect originalStackFrame;
@property(nonatomic) BOOL resizedStack;
@property(nonatomic,copy) void (^toggle)(void);
- (void)scanWindows:(NSArray<UIWindow *> *)windows enabled:(BOOL)enabled hidden:(BOOL)hidden;
- (void)detach;
@end
@implementation TT51SheetEye
- (void)pressed:(id)sender {
    if(self.toggle)self.toggle();
}
- (void)detach {
    if(self.stack){
        [self.stack removeArrangedSubview:self.row];
        if(self.stackHeight)self.stackHeight.constant=self.originalHeight;
        if(self.resizedStack)self.stack.frame=self.originalStackFrame;
    }
    [self.row removeFromSuperview];
    if(self.scroll) {
        UIEdgeInsets inset=self.scroll.contentInset;
        // Remove only our reserved space; preserve changes made by the host.
        CGPoint offset=self.scroll.contentOffset;offset.y+=self.addedInset;
        inset.top-=self.addedInset;self.scroll.contentInset=inset;
        [self.scroll setContentOffset:offset animated:NO];
        UIEdgeInsets indicators=self.scroll.verticalScrollIndicatorInsets;
        indicators.top-=self.addedInset;self.scroll.verticalScrollIndicatorInsets=indicators;
    }
    self.row=nil;self.nativeRow=nil;self.scope=nil;self.stack=nil;self.scroll=nil;self.addedInset=0;self.stackHeight=nil;self.resizedStack=NO;
}
- (TT51EyeRow *)makeRow:(UIView *)native label:(UILabel *)label {
    TT51EyeRow *row=[TT51EyeRow buttonWithType:UIButtonTypeCustom];
    row.frame=native.bounds;row.accessibilityLabel=@"إخفاء/إظهار الواجهة";
    row.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
    row.caption=[UILabel new];row.caption.text=row.accessibilityLabel;
    row.caption.font=label.font ?: [UIFont systemFontOfSize:16 weight:UIFontWeightSemibold];
    row.caption.textColor=label.textColor ?: UIColor.labelColor;
    row.caption.textAlignment=label?label.textAlignment:NSTextAlignmentLeft;
    row.caption.adjustsFontSizeToFitWidth=YES;row.caption.minimumScaleFactor=.8;
    row.caption.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
    row.glyph=[[UIImageView alloc] initWithImage:[UIImage systemImageNamed:@"eye"]];
    row.glyph.contentMode=UIViewContentModeScaleAspectFit;row.glyph.tintColor=row.caption.textColor;
    CGFloat w=native.bounds.size.width;
    CGRect text=label?[label convertRect:label.bounds toView:native]:CGRectMake(52,0,w-70,native.bounds.size.height);
    row.textX=text.origin.x;row.textWidth=text.size.width;row.iconX=16;
    for(UIView *v in TT51Descendants(native))if([v isKindOfClass:UIImageView.class] &&
        v.bounds.size.width>=16 && v.bounds.size.width<=40) {
        row.iconX=[v convertRect:v.bounds toView:native].origin.x;break;
    }
    UIColor *background=nil;
    for(UIView *p=native;p && p!=native.window;p=p.superview)if(p.backgroundColor && CGColorGetAlpha(p.backgroundColor.CGColor)>.9){background=p.backgroundColor;break;}
    row.backgroundColor=background ?: UIColor.secondarySystemGroupedBackgroundColor;
    [row addSubview:row.caption];[row addSubview:row.glyph];
    [row addTarget:self action:@selector(pressed:) forControlEvents:UIControlEventTouchUpInside];
    return row;
}
- (void)scanWindows:(NSArray<UIWindow *> *)windows enabled:(BOOL)enabled hidden:(BOOL)hidden {
    if(!enabled){[self detach];return;}
    if(self.row && TT51Visible(self.nativeRow) && TT51Visible(self.scope)) {
        [self.row.glyph setImage:[UIImage systemImageNamed:hidden?@"eye.slash":@"eye"]];
        if(self.scroll) {
            CGRect r=[self.nativeRow convertRect:self.nativeRow.bounds toView:self.scroll];
            self.row.frame=CGRectMake(r.origin.x,r.origin.y-r.size.height,r.size.width,r.size.height);
        }
        return;
    }
    [self detach];
    for(UIWindow *window in windows)for(UIView *leaf in TT51Descendants(window)) {
        if(!TT51Visible(leaf) || !TT51Download(leaf))continue;
        UIView *native=nil;
        // The first wide compact ancestor is the actual action row, not its whole card.
        for(UIView *p=leaf;p && p!=window;p=p.superview) {
            CGRect f=[p convertRect:p.bounds toView:window];
            if(f.size.width>window.bounds.size.width*.65 && f.size.height>=40 && f.size.height<=86){native=p;break;}
        }
        if(!native)continue;
        CGRect screen=[native convertRect:native.bounds toView:window];
        if(screen.origin.y<window.bounds.size.height*.2)continue;
        UIView *scope=nil;
        for(UIView *p=native.superview;p && p!=window;p=p.superview) {
            if(TT51SheetPeers(p)){scope=p;break;}
            if(p.bounds.size.height>window.bounds.size.height*.85)break;
        }
        if(!scope)continue; // A download menu or profile entry is not the video action sheet.
        UILabel *label=[leaf isKindOfClass:UILabel.class]?(UILabel *)leaf:nil;
        if(!label)for(UIView *v in TT51Descendants(native))if([v isKindOfClass:UILabel.class]){label=(UILabel *)v;break;}
        TT51EyeRow *row=[self makeRow:native label:label];
        UIView *branch=native;UIStackView *stack=nil;
        for(UIView *p=native.superview;p && p!=window;p=p.superview) {
            if([p isKindOfClass:UIStackView.class] && ((UIStackView *)p).axis==UILayoutConstraintAxisVertical &&
               [((UIStackView *)p).arrangedSubviews containsObject:branch]){stack=(UIStackView *)p;break;}
            if(p==scope || p.bounds.size.height>100)break;
            branch=p;
        }
        if(stack) {
            NSUInteger index=[stack.arrangedSubviews indexOfObject:branch];
            if(index==NSNotFound)continue;
            row.translatesAutoresizingMaskIntoConstraints=NO;
            [row.heightAnchor constraintEqualToConstant:screen.size.height].active=YES;
            // Grow an explicit stack height if present; otherwise intrinsic stack sizing propagates normally.
            for(NSLayoutConstraint *c in stack.constraints)
                if(c.active && c.firstItem==stack && c.firstAttribute==NSLayoutAttributeHeight && !c.secondItem && c.relation==NSLayoutRelationEqual){
                    self.stackHeight=c;self.originalHeight=c.constant;c.constant+=screen.size.height;break;
                }
            if(!self.stackHeight && stack.translatesAutoresizingMaskIntoConstraints){
                self.resizedStack=YES;self.originalStackFrame=stack.frame;
                CGRect f=stack.frame;f.size.height+=screen.size.height;stack.frame=f;
            }
            [stack insertArrangedSubview:row atIndex:index];self.stack=stack;
        } else {
            // Table/collection/custom-scroll sheets: reserve a new first row without altering data sources,
            // native cell frames, gesture targets or item indices. Existing content remains scrollable.
            UIScrollView *scroll=nil;
            for(UIView *p=native.superview;p && p!=window;p=p.superview)
                if([p isKindOfClass:UIScrollView.class] && p.bounds.size.width>window.bounds.size.width*.65 &&
                   p.bounds.size.height>screen.size.height*2){scroll=(UIScrollView *)p;break;}
            if(!scroll)continue;
            CGRect content=[native convertRect:native.bounds toView:scroll];
            CGFloat top=scroll.contentOffset.y+scroll.adjustedContentInset.top;
            // Only augment the first card, never a Download row scrolled into view on a different screen.
            if(content.origin.y-top>100 || content.origin.y-top< -10)continue;
            self.oldInset=scroll.contentInset;self.oldIndicators=scroll.verticalScrollIndicatorInsets;
            self.addedInset=screen.size.height;
            UIEdgeInsets inset=scroll.contentInset;inset.top+=self.addedInset;
            CGPoint offset=scroll.contentOffset;offset.y-=self.addedInset;
            scroll.contentInset=inset;
            UIEdgeInsets indicators=scroll.verticalScrollIndicatorInsets;indicators.top+=self.addedInset;
            scroll.verticalScrollIndicatorInsets=indicators;[scroll setContentOffset:offset animated:NO];
            row.frame=CGRectMake(content.origin.x,content.origin.y-screen.size.height,content.size.width,screen.size.height);
            row.layer.cornerRadius=12;row.layer.maskedCorners=kCALayerMinXMinYCorner|kCALayerMaxXMinYCorner;
            [scroll addSubview:row];self.scroll=scroll;
        }
        self.row=row;self.nativeRow=native;self.scope=scope;
        [self.row.glyph setImage:[UIImage systemImageNamed:hidden?@"eye.slash":@"eye"]];
        [row setNeedsLayout];[row layoutIfNeeded];return;
    }
}
@end

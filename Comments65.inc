// 65: optional "Cards" look for the native comment list. Observed classes only
// (AWECommentPanelCell inside its UITableView). Pure visuals: a background layer
// behind the cell content, gray gaps, hidden hairline. No controls are added,
// moved or retargeted; avatars, like/reply/dislike, text, stickers, footers,
// gestures and the composer are untouched. Original colors/alpha are restored.
static NSString *const JK65Key=@"tt.comments.style";
static const void *kJK65Cell=&kJK65Cell,*kJK65Table=&kJK65Table;
static BOOL JK65On(void){return [[NSUserDefaults standardUserDefaults] integerForKey:JK65Key]==1;}
@interface JK65Saved:NSObject
@property(nonatomic,strong) UIColor *cellBg,*contentBg,*tableBg;
@property(nonatomic,strong) CALayer *card;
@property(nonatomic,weak) UIView *divider;
@property(nonatomic) CGFloat dividerAlpha;
@property(nonatomic) BOOL dark;
@end
@implementation JK65Saved
@end
static BOOL JK65IsDark(UIColor *base,UITraitCollection *traits){
    UIColor *c=base?[base resolvedColorWithTraitCollection:traits]:nil;CGFloat r=0,g=0,b=0,a=0;
    if(c&&[c getRed:&r green:&g blue:&b alpha:&a]&&a>.05)return (.299*r+.587*g+.114*b)<.5;
    return traits.userInterfaceStyle==UIUserInterfaceStyleDark;
}
static UIColor *JK65Gray(BOOL dark){return dark?[UIColor colorWithRed:.043 green:.043 blue:.051 alpha:1]:[UIColor colorWithRed:.949 green:.953 blue:.961 alpha:1];}
static UIColor *JK65CardColor(BOOL dark){return dark?[UIColor colorWithRed:.106 green:.106 blue:.122 alpha:1]:UIColor.whiteColor;}
static UITableView *JK65Table(UIView *cell){
    UIView *v=cell.superview;for(int i=0;v&&i<4;i++,v=v.superview)if([v isKindOfClass:UITableView.class])return (UITableView *)v;
    return nil;
}
static void JK65Restore(UITableViewCell *cell){
    JK65Saved *s=objc_getAssociatedObject(cell,kJK65Cell);if(!s)return;
    [s.card removeFromSuperlayer];
    cell.backgroundColor=s.cellBg;cell.contentView.backgroundColor=s.contentBg;
    if(s.divider)s.divider.alpha=s.dividerAlpha;
    objc_setAssociatedObject(cell,kJK65Cell,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    UITableView *t=JK65Table(cell);JK65Saved *ts=t?objc_getAssociatedObject(t,kJK65Table):nil;
    if(ts){t.backgroundColor=ts.tableBg;objc_setAssociatedObject(t,kJK65Table,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
}
static void JK65Apply(UITableViewCell *cell){
    if(!NSThread.isMainThread||![cell isKindOfClass:UITableViewCell.class])return;
    if(!JK65On()){JK65Restore(cell);return;}
    UIView *content=cell.contentView;if(!content||content.bounds.size.width<120||content.bounds.size.height<40)return;
    JK65Saved *s=objc_getAssociatedObject(cell,kJK65Cell);
    if(!s){
        s=[JK65Saved new];s.cellBg=cell.backgroundColor;s.contentBg=content.backgroundColor;
        s.dark=JK65IsDark(s.contentBg?:s.cellBg,cell.traitCollection);
        objc_setAssociatedObject(cell,kJK65Cell,s,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    UITableView *t=JK65Table(cell);
    if(t&&!objc_getAssociatedObject(t,kJK65Table)){
        JK65Saved *ts=[JK65Saved new];ts.tableBg=t.backgroundColor;objc_setAssociatedObject(t,kJK65Table,ts,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    UIColor *gray=JK65Gray(s.dark);
    if(t&&![t.backgroundColor isEqual:gray])t.backgroundColor=gray;
    if(![cell.backgroundColor isEqual:gray])cell.backgroundColor=gray;
    if(![content.backgroundColor isEqual:gray])content.backgroundColor=gray;
    [CATransaction begin];[CATransaction setDisableActions:YES];
    if(!s.card){s.card=[CALayer layer];s.card.name=@"jk65.card";}
    if(s.card.superlayer!=content.layer||[content.layer.sublayers firstObject]!=s.card)[content.layer insertSublayer:s.card atIndex:0];
    CGRect r=CGRectInset(content.bounds,10,3);
    s.card.frame=r;s.card.cornerRadius=14;s.card.backgroundColor=JK65CardColor(s.dark).CGColor;
    s.card.borderWidth=.5;s.card.borderColor=(s.dark?[UIColor colorWithWhite:1 alpha:.08]:[UIColor colorWithWhite:0 alpha:.06]).CGColor;
    s.card.shadowColor=UIColor.blackColor.CGColor;s.card.shadowOpacity=s.dark?0:.06;s.card.shadowRadius=4;s.card.shadowOffset=CGSizeMake(0,1);
    s.card.shadowPath=[UIBezierPath bezierPathWithRoundedRect:s.card.bounds cornerRadius:14].CGPath;
    [CATransaction commit];
    // Native 0.5pt hairline between rows: dimmed only while cards are on.
    for(UIView *v in content.subviews){
        if(v.bounds.size.height<=1&&v.bounds.size.width>=content.bounds.size.width*.8){
            if(s.divider!=v){s.divider=v;s.dividerAlpha=v.alpha;}
            if(v.alpha!=0)v.alpha=0;break;
        }
    }
}
// Visible comment cells re-layout after the picker changes (bounded walk).
static void JK65Refresh(void){
    for(UIWindow *w in UIApplication.sharedApplication.windows){
        NSMutableArray *q=[NSMutableArray arrayWithObject:w];
        for(NSUInteger i=0;i<q.count&&i<4000;i++){
            UIView *v=q[i];if([v isKindOfClass:UITableViewCell.class]){JK65Apply((UITableViewCell *)v);continue;}
            [q addObjectsFromArray:v.subviews];
        }
    }
}
@interface JK65Picker:NSObject
@property(nonatomic,weak) UILabel *note;
@end
@implementation JK65Picker
- (void)changed:(UISegmentedControl *)s {
    [[NSUserDefaults standardUserDefaults] setInteger:s.selectedSegmentIndex forKey:JK65Key];
    self.note.text=s.selectedSegmentIndex==1?@"كروت: خلفية بطاقات للتعليقات الأصلية فقط. كل الأزرار والإجراءات تبقى أصلية.":@"تعليقات TikTok الأصلية.";
    JK65Refresh();
}
@end
static void JK65AddPicker(UIStackView *stack){
    UILabel *title=[UILabel new];title.text=@"نمط التعليقات";title.textColor=[UIColor colorWithWhite:1 alpha:.88];
    title.font=[UIFont systemFontOfSize:13 weight:UIFontWeightSemibold];title.textAlignment=NSTextAlignmentLeft;
    [stack addArrangedSubview:title];
    UISegmentedControl *seg=[[UISegmentedControl alloc] initWithItems:@[@"TikTok",@"كروت"]];
    seg.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
    seg.selectedSegmentIndex=JK65On()?1:0;seg.accessibilityLabel=@"نمط التعليقات";
    [seg.heightAnchor constraintEqualToConstant:42].active=YES;
    seg.backgroundColor=[UIColor colorWithWhite:1 alpha:.08];
    seg.selectedSegmentTintColor=[UIColor colorWithRed:.80 green:.74 blue:.60 alpha:.32];
    [seg setTitleTextAttributes:@{NSForegroundColorAttributeName:UIColor.whiteColor,NSFontAttributeName:[UIFont systemFontOfSize:11 weight:UIFontWeightSemibold]} forState:UIControlStateNormal];
    seg.layer.cornerRadius=12;seg.clipsToBounds=YES;
    JK65Picker *p=[JK65Picker new];UILabel *note=[UILabel new];note.numberOfLines=0;note.font=[UIFont systemFontOfSize:12];
    note.textColor=[UIColor colorWithWhite:1 alpha:.74];note.textAlignment=NSTextAlignmentLeft;
    note.text=JK65On()?@"كروت: خلفية بطاقات للتعليقات الأصلية فقط. كل الأزرار والإجراءات تبقى أصلية.":@"تعليقات TikTok الأصلية.";
    p.note=note;objc_setAssociatedObject(seg,"jk65.picker",p,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    [seg addTarget:p action:@selector(changed:) forControlEvents:UIControlEventValueChanged];
    [stack addArrangedSubview:seg];[stack addArrangedSubview:note];
}

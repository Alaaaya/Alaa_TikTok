UIView *sheet=[[UIView alloc] initWithFrame:CGRectMake(0,300,self.window.bounds.size.width,500)];
sheet.backgroundColor=UIColor.systemGroupedBackgroundColor;
UIStackView *sheetStack=[[UIStackView alloc] initWithFrame:CGRectMake(14,25,sheet.bounds.size.width-28,168)];
sheetStack.backgroundColor=UIColor.secondarySystemGroupedBackgroundColor;sheetStack.layer.cornerRadius=12;
sheetStack.axis=UILayoutConstraintAxisVertical;sheetStack.distribution=UIStackViewDistributionFill;sheetStack.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
[sheet addSubview:sheetStack];
NSMutableArray<UIView *> *nativeRows=[NSMutableArray new];
for(NSString *name in @[@"تنزيل",@"غير مهتم",@"إبلاغ"]) {
    UIButton *r=[UIButton buttonWithType:UIButtonTypeCustom];r.frame=CGRectMake(0,0,sheetStack.bounds.size.width,56);
    r.backgroundColor=UIColor.clearColor;
    [r.heightAnchor constraintEqualToConstant:56].active=YES;
    UILabel *l=[[UILabel alloc] initWithFrame:CGRectMake(18,0,r.bounds.size.width-70,56)];
    l.text=name;l.font=[UIFont systemFontOfSize:17 weight:UIFontWeightSemibold];l.textColor=UIColor.labelColor;l.textAlignment=NSTextAlignmentRight;
    UIImageView *im=[[UIImageView alloc] initWithImage:[UIImage systemImageNamed:@"arrow.down.to.line"]];
    im.frame=CGRectMake(r.bounds.size.width-40,16,24,24);[r addSubview:l];[r addSubview:im];
    [sheetStack addArrangedSubview:r];[nativeRows addObject:r];
}
{UISwitch *nativeSwitch=[UISwitch new];nativeSwitch.onTintColor=UIColor.systemGreenColor;nativeSwitch.frame=CGRectMake(16,12,51,31);[nativeRows[2] addSubview:nativeSwitch];}
[self.window addSubview:sheet];[sheetStack layoutIfNeeded];
CGFloat originalStackHeight=sheetStack.bounds.size.height;
CGFloat addedRowHeight=nativeRows[0].bounds.size.height;
TT51SheetEye *sheetEye=[TT51SheetEye new];__block BOOL eyeHidden=NO;
sheetEye.toggle=^{eyeHidden=!eyeHidden;};
[sheetEye scanWindows:@[self.window] enabled:YES hidden:NO];[sheetStack layoutIfNeeded];
Check(sheetEye.row && sheetStack.arrangedSubviews[0]==sheetEye.row && sheetStack.arrangedSubviews[1]==sheetEye.row2 && sheetStack.arrangedSubviews[2]==nativeRows[0],@"51 eye and Joker download rows in the same native stack before Download");
Check(sheetEye.row.backgroundColor==UIColor.clearColor && sheetEye.row2.backgroundColor==UIColor.clearColor && sheetEye.row.superview==nativeRows[0].superview,@"52 rows share the native card, no separate background card or gap");
{
    CGFloat sw=sheetStack.bounds.size.width;
    sheetEye.row.frame=CGRectMake(0,0,sw,56);[sheetEye.row layoutIfNeeded];
    Check([sheetEye.row.toggleSwitch.onTintColor isEqual:UIColor.systemGreenColor],@"52 switch copies native auto-scroll switch style");
    Check(sheetEye.row.toggleSwitch && CGRectGetMinX(sheetEye.row.toggleSwitch.frame)<30 && sheetEye.row.toggleSwitch.frame.origin.x<sheetEye.row.caption.frame.origin.x,@"52 switch physically at left");
    CGSize need=[sheetEye.row.caption.text sizeWithAttributes:@{NSFontAttributeName:sheetEye.row.caption.font}];
    Check(sheetEye.row.caption.bounds.size.width>=need.width && [sheetEye.row.caption.text isEqual:@"إخفاء الواجهة"] && sheetEye.row.caption.textAlignment==NSTextAlignmentRight,@"52 caption not truncated, exact title");
    Check([sheetEye.row2.caption.text isEqual:@"تنزيل Joker"],@"52 Joker download row title");
}

Check(sheetEye.row.glyph.frame.origin.x>sheetEye.row.caption.frame.origin.x,@"51 native icon column preserved");
[sheetEye.row sendActionsForControlEvents:UIControlEventTouchUpInside];Check(eyeHidden,@"51 row hides UI");
[sheetEye.row sendActionsForControlEvents:UIControlEventTouchUpInside];Check(!eyeHidden,@"51 same row restores UI");
sheetEye.row.toggleSwitch.on=YES;[sheetEye.row.toggleSwitch sendActionsForControlEvents:UIControlEventValueChanged];Check(eyeHidden,@"52 switch toggles eye hide");
[sheetEye scanWindows:@[self.window] enabled:YES hidden:YES];Check(sheetEye.row.toggleSwitch.on,@"52 switch reflects eye state");
sheetEye.row.toggleSwitch.on=NO;[sheetEye.row.toggleSwitch sendActionsForControlEvents:UIControlEventValueChanged];Check(!eyeHidden,@"52 switch restores");
[sheetEye scanWindows:@[self.window] enabled:YES hidden:NO];Check(!sheetEye.row.toggleSwitch.on,@"52 switch off when UI shown");
__block NSInteger lastCode=-1;sheetEye.optionAction=^(NSInteger c){lastCode=c;};
CGFloat rowBefore=sheetEye.row2.bounds.size.height;
[sheetEye.row2 sendActionsForControlEvents:UIControlEventTouchUpInside];
Check(sheetEye.subRows.count==7,@"54 download options expand in the sheet");
[(UIControl *)sheetEye.subRows[1] sendActionsForControlEvents:UIControlEventTouchUpInside];Check(lastCode==1,@"54 HD option dispatches");
[(UIControl *)sheetEye.subRows[6] sendActionsForControlEvents:UIControlEventTouchUpInside];Check(sheetEye.subRows==nil && sheetEye.row2.caption.alpha==1,@"54 back collapses");
(void)rowBefore;
[sheetEye scanWindows:@[self.window] enabled:YES hidden:YES];Check(sheetStack.arrangedSubviews.count==5,@"51 no duplicate insertion");
NSLog(@"51 geometry before=%f row=%f after=%f",originalStackHeight,addedRowHeight,sheetStack.bounds.size.height);
Check(sheetStack.bounds.size.height>=originalStackHeight+2*addedRowHeight-.5,@"51 expands fixed-frame stack to fit fourth row");
Check(CGRectGetMaxY(sheetEye.row2.frame)<=CGRectGetMinY(nativeRows[0].frame)+.01,@"51 stack rows do not overlap");
ShotView(self.window,@"51-native-sheet-stack");
[sheetEye scanWindows:@[self.window] enabled:NO hidden:NO];
Check(sheetStack.arrangedSubviews.count==3 && sheetStack.arrangedSubviews[0]==nativeRows[0],@"51 disabled restores native rows");
[sheet removeFromSuperview];
// Scroll-based native cards keep all original items and gestures.
UIScrollView *sheetScroll=[[UIScrollView alloc] initWithFrame:CGRectMake(0,300,self.window.bounds.size.width,450)];
sheetScroll.backgroundColor=UIColor.systemGroupedBackgroundColor;sheetScroll.contentSize=CGSizeMake(sheetScroll.bounds.size.width,650);
UIView *card=[[UIView alloc] initWithFrame:CGRectMake(14,18,sheetScroll.bounds.size.width-28,168)];card.backgroundColor=UIColor.secondarySystemGroupedBackgroundColor;
for(NSUInteger i=0;i<nativeRows.count;i++){
    UIView *r=nativeRows[i];[r removeFromSuperview];r.translatesAutoresizingMaskIntoConstraints=YES;r.frame=CGRectMake(0,i*56,card.bounds.size.width,56);[card addSubview:r];
}
[sheetScroll addSubview:card];[self.window addSubview:sheetScroll];
UIEdgeInsets initialInset=sheetScroll.contentInset;CGRect firstFrame=nativeRows[0].frame;
[sheetEye scanWindows:@[self.window] enabled:YES hidden:NO];
Check(sheetEye.row && sheetEye.scroll==sheetScroll,@"51 discovers custom scroll native sheet");
Check(fabs(sheetScroll.contentInset.top-initialInset.top-112)<.01,@"51 reserves row instead of covering Download");
Check(CGRectEqualToRect(firstFrame,nativeRows[0].frame),@"51 native cell frames untouched");
Check(CGRectGetMaxY(sheetEye.row2.frame)<=CGRectGetMinY([nativeRows[0] convertRect:nativeRows[0].bounds toView:sheetScroll])+.1,@"51 scroll eye above Download");
ShotView(self.window,@"51-native-sheet-scroll");
[sheetEye detach];Check(fabs(sheetScroll.contentInset.top-initialInset.top)<.01,@"51 removes only added inset");
[sheetScroll removeFromSuperview];
// Symbol fallback works without a translated Download label.
UIImageView *symbol=[UIImageView new];symbol.accessibilityIdentifier=@"arrow.down.to.line";Check(TT51Download(symbol),@"51 explicit download icon fallback");
UILabel *unrelated=[UILabel new];unrelated.text=@"Download HD";Check(!TT51Download(unrelated),@"51 own download menu excluded");
Check(!TT51SheetPeers(unrelated),@"51 no insertion without native Report and Not Interested peers");

// 55: sheet WITHOUT a native Download row still gets Joker rows; small viewport scrolls to the expanded card top automatically.
{
    UIScrollView *vs=[[UIScrollView alloc] initWithFrame:CGRectMake(0,420,self.window.bounds.size.width,330)];
    vs.backgroundColor=UIColor.systemGroupedBackgroundColor;
    UIView *content=[[UIView alloc] initWithFrame:CGRectMake(0,0,vs.bounds.size.width,900)];[vs addSubview:content];vs.contentSize=content.bounds.size;
    UIStackView *card2=[[UIStackView alloc] initWithFrame:CGRectMake(14,500,vs.bounds.size.width-28,112)];
    card2.axis=UILayoutConstraintAxisVertical;card2.backgroundColor=UIColor.secondarySystemGroupedBackgroundColor;card2.layer.cornerRadius=12;
    card2.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;[content addSubview:card2];
    for(NSString *name in @[@"غير مهتم",@"إبلاغ"]){
        UIButton *r=[UIButton buttonWithType:UIButtonTypeCustom];r.frame=CGRectMake(0,0,card2.bounds.size.width,56);[r.heightAnchor constraintEqualToConstant:56].active=YES;
        UILabel *l=[[UILabel alloc] initWithFrame:CGRectMake(18,0,r.bounds.size.width-70,56)];l.text=name;l.textAlignment=NSTextAlignmentRight;[r addSubview:l];
        UIImageView *im=[[UIImageView alloc] initWithImage:[UIImage systemImageNamed:@"flag"]];im.frame=CGRectMake(r.bounds.size.width-40,16,24,24);[r addSubview:im];
        [card2 addArrangedSubview:r];
    }
    [self.window addSubview:vs];[card2 layoutIfNeeded];
    vs.contentOffset=CGPointMake(0,vs.contentSize.height-vs.bounds.size.height);
    TT51SheetEye *se2=[TT51SheetEye new];
    [se2 scanWindows:@[self.window] enabled:YES hidden:NO];[card2 layoutIfNeeded];
    Check(se2.row && card2.arrangedSubviews[0]==se2.row && card2.arrangedSubviews[1]==se2.row2,@"55 Joker rows inserted without native Download row");
    vs.contentOffset=CGPointMake(0,vs.contentSize.height-vs.bounds.size.height);
    [se2.row2 sendActionsForControlEvents:UIControlEventTouchUpInside];
    CFRunLoopRunInMode(kCFRunLoopDefaultMode,.6,false);
    CGRect rr=[se2.row2 convertRect:se2.row2.bounds toView:vs];
    CGFloat topVis=rr.origin.y-vs.contentOffset.y;
    NSLog(@"55 dbg topVis=%f off=%f rr=%@ cs=%f mode=%ld",topVis,vs.contentOffset.y,NSStringFromCGRect(rr),vs.contentSize.height,(long)se2.mode);
    Check(topVis>=-.5 && topVis<=40,@"55 expanded card top visible automatically after tap (started scrolled down)");
    Check(card2.layer.cornerRadius>=12 && card2.layer.maskedCorners==(kCALayerMinXMinYCorner|kCALayerMaxXMinYCorner|kCALayerMinXMaxYCorner|kCALayerMaxXMaxYCorner),@"55 card keeps all four rounded corners");
    ShotView(self.window,@"55-expanded-reveal");
    [(UIControl *)se2.subRows[se2.subRows.count-1] sendActionsForControlEvents:UIControlEventTouchUpInside];
    Check(se2.mode==0 || se2.mode==1,@"55 back works");
    [se2 scanWindows:@[self.window] enabled:NO hidden:NO];
    Check(card2.arrangedSubviews.count==2 && CGRectGetHeight(card2.bounds)>=111,@"55 restored original geometry");
    [vs removeFromSuperview];
}

// 55 visual fixture: native-like autolayout sheet, gray background, Joker card + peer card below with gap.
{
    UIView *cover=[[UIView alloc] initWithFrame:self.window.bounds];cover.backgroundColor=UIColor.systemGroupedBackgroundColor;[self.window addSubview:cover];
    UIScrollView *vs=[[UIScrollView alloc] initWithFrame:CGRectMake(0,200,self.window.bounds.size.width,674)];vs.backgroundColor=UIColor.systemGroupedBackgroundColor;[self.window addSubview:vs];
    UIStackView *outer=[[UIStackView alloc] initWithFrame:CGRectMake(14,12,vs.bounds.size.width-28,0)];outer.axis=UILayoutConstraintAxisVertical;outer.spacing=10;outer.translatesAutoresizingMaskIntoConstraints=NO;[vs addSubview:outer];
    UIView *holder=[[UIView alloc] initWithFrame:CGRectMake(0,0,vs.bounds.size.width,900)];[vs addSubview:holder];vs.contentSize=holder.bounds.size;[outer removeFromSuperview];[holder addSubview:outer];[NSLayoutConstraint activateConstraints:@[[outer.topAnchor constraintEqualToAnchor:holder.topAnchor constant:12],[outer.leadingAnchor constraintEqualToAnchor:holder.leadingAnchor constant:14],[outer.widthAnchor constraintEqualToConstant:vs.bounds.size.width-28]]];
    NSMutableArray *cards=[NSMutableArray new];
    for(NSArray *names in @[@[@"غير مهتم",@"إبلاغ"],@[@"السرعة",@"شاشة خالية",@"الشرح"]]){
        UIStackView *c=[UIStackView new];c.axis=UILayoutConstraintAxisVertical;c.backgroundColor=UIColor.secondarySystemGroupedBackgroundColor;c.layer.cornerRadius=12;c.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
        for(NSString *n in names){UIButton *b=[UIButton buttonWithType:UIButtonTypeCustom];[b.heightAnchor constraintEqualToConstant:56].active=YES;
            UILabel *l=[[UILabel alloc] initWithFrame:CGRectMake(18,0,vs.bounds.size.width-100,56)];l.text=n;l.textAlignment=NSTextAlignmentRight;[b addSubview:l];[c addArrangedSubview:b];}
        [outer addArrangedSubview:c];[cards addObject:c];
    }
    [vs layoutIfNeeded];
    TT51SheetEye *se3=[TT51SheetEye new];[se3 scanWindows:@[self.window] enabled:YES hidden:NO];[vs layoutIfNeeded];
    Check(se3.row && se3.cardHost==cards[0],@"55 card host is the native white card, not a row");
    [se3.row2 sendActionsForControlEvents:UIControlEventTouchUpInside];[vs layoutIfNeeded];
    CFRunLoopRunInMode(kCFRunLoopDefaultMode,.5,false);[vs layoutIfNeeded];
    UIView *c0=cards[0],*c1=cards[1];
    CGRect f0=[c0 convertRect:c0.bounds toView:vs],f1=[c1 convertRect:c1.bounds toView:vs];
    Check(f1.origin.y-CGRectGetMaxY(f0)>=9.5,@"55 native gap below Joker card preserved (autolayout)");
    Check(c0.layer.cornerRadius>=12 && c0.layer.masksToBounds && c1.layer.cornerRadius>=12,@"55 both cards rounded");
    NSLog(@"55 gapdbg c0=%@ c1=%@ outer=%@ vs=%@ win=%@ sub=%lu",NSStringFromCGRect(f0),NSStringFromCGRect(f1),NSStringFromCGRect(outer.frame),NSStringFromCGRect(vs.frame),NSStringFromCGRect(self.window.bounds),(unsigned long)self.window.subviews.count);
    NSLog(@"55 cdbg r=%f m=%d mc=%lu host=%@ c0=%@ sub0=%@ bg=%@",c0.layer.cornerRadius,c0.layer.masksToBounds,(unsigned long)c0.layer.maskedCorners,se3.cardHost,c0,c0.subviews.firstObject,se3.row2.backgroundColor);
    ShotView(self.window,@"55-card-gap");
    [se3 scanWindows:@[self.window] enabled:NO hidden:NO];[vs removeFromSuperview];[cover removeFromSuperview];
}
NSLog(@"PASS 51 native sheet stack+scroll row, exact Arabic title, two-way eye, native frames, duplicate prevention, cleanup and symbol fallback");

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
[sheetEye.subRows[1] sendActionsForControlEvents:UIControlEventTouchUpInside];Check(lastCode==1,@"54 HD option dispatches");
[sheetEye.subRows[6] sendActionsForControlEvents:UIControlEventTouchUpInside];Check(sheetEye.subRows==nil && sheetEye.row2.caption.alpha==1,@"54 back collapses");
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
NSLog(@"PASS 51 native sheet stack+scroll row, exact Arabic title, two-way eye, native frames, duplicate prevention, cleanup and symbol fallback");

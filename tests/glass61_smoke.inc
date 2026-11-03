{
    [vc showCategory:7 animated:NO];[vc.view layoutIfNeeded];
    Check(vc.category==7&&[vc.pageTitle.text isEqual:@"الزجاج"],@"61 glass section Arabic and reachable");
    UISlider *tint=(id)[vc.stack viewWithTag:610],*blur=(id)[vc.stack viewWithTag:611],*edge=(id)[vc.stack viewWithTag:612];
    Check(tint&&blur&&edge&&tint.semanticContentAttribute==UISemanticContentAttributeForceLeftToRight,@"61 bounded physical LTR sliders");
    Shot(vc,@"61-glass-default");
    UIView *native=[UIView new];native.backgroundColor=UIColor.orangeColor;native.alpha=.7;native.layer.borderWidth=3;
    NSMutableArray *owned=[NSMutableArray new];
    for(NSString *role in @[@"floating-clear",@"rail-decor",@"alaa-dock",@"sheet-card",@"panel"]){
        UIVisualEffectView *v=[[UIVisualEffectView alloc] initWithEffect:[UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemThinMaterialDark]];v.frame=CGRectMake(0,0,100,40);v.contentView.backgroundColor=[UIColor colorWithWhite:0 alpha:.3];JK61Register(v,v.contentView,role);[owned addObject:v];
    }
    NSArray *rows=[vc.stack.arrangedSubviews copy];
    tint.value=.9;[tint sendActionsForControlEvents:UIControlEventValueChanged];blur.value=.3;[blur sendActionsForControlEvents:UIControlEventValueChanged];edge.value=.8;[edge sendActionsForControlEvents:UIControlEventValueChanged];
    Check([rows isEqual:vc.stack.arrangedSubviews]&&tint==[vc.stack viewWithTag:610],@"61 live preview never rebuilds slider controls mid-drag");
    for(UIView *v in owned){JK61GlassRecord *r=[JK61Glasses() objectForKey:v];Check(v.layer.borderWidth>0&&r.animator&&fabs(r.animator.fractionComplete-.3)<.001&&CGColorGetAlpha(r.tint.backgroundColor.CGColor)>.3,@"61 every explicit owned surface receives tint/blur/edge live");}
    Check([native.backgroundColor isEqual:UIColor.orangeColor]&&native.alpha==.7&&native.layer.borderWidth==3&&![JK61Glasses() objectForKey:native],@"61 native UIKit/media views never registered or modified");
    Check(vc.panel.layer.borderWidth>0&&[JK61Glasses() objectForKey:vc.panel].animator,@"61 open settings panel itself is live preview");Shot(vc,@"61-glass-varied");
    // Fire actual debounce to model process relaunch, not a direct defaults write.
    [[JK61Persistence shared].timer fire];JK61Values=nil;Check(fabs(JK61Value(@"tint")-.9)<.001&&fabs(JK61Value(@"blur")-.3)<.001,@"61 debounced values reload after in-memory cache recreation");
    // New Joker-owned surfaces inherit current values. Native views still do not.
    UIVisualEffectView *fresh=[[UIVisualEffectView alloc] initWithEffect:[UIBlurEffect effectWithStyle:UIBlurEffectStyleDark]];JK61Register(fresh,nil,@"sheet-card");Check(fresh.layer.borderWidth>0&&[JK61Glasses() objectForKey:fresh].animator,@"61 later sheet inherits persisted values");
    [vc glassReset:nil];Check(tint.value==.5&&blur.value==1&&edge.value==0&&!JK61Customized(),@"61 reset synchronizes all sliders to baseline");
    for(UIView *v in owned){JK61GlassRecord *r=[JK61Glasses() objectForKey:v];Check(v.layer.borderWidth==0&&!r.animator&&[(UIVisualEffectView *)v effect]==r.effect&&[r.tint.backgroundColor isEqual:r.tintColor],@"61 reset restores exact glass baseline without retained animator");}
    Shot(vc,@"61-glass-reset");
    [vc showCategory:0 animated:NO];UISwitch *floating=(id)[vc.stack viewWithTag:261],*emoji=(id)[vc.stack viewWithTag:246];
    Check(floating&&emoji&&!floating.on,@"61 readable main-feed floating switch defaults OFF");floating.on=YES;[floating sendActionsForControlEvents:UIControlEventValueChanged];emoji.on=YES;[emoji sendActionsForControlEvents:UIControlEventValueChanged];
    [vc showCategory:-1 animated:NO];[vc showCategory:0 animated:NO];Check(((UISwitch *)[vc.stack viewWithTag:261]).on&&((UISwitch *)[vc.stack viewWithTag:246]).on,@"61 both independent switches persist on reopen");
    Shot(vc,@"61-floating-toggle");[D() setBool:NO forKey:@"tt61.feedFloating"];[D() setBool:NO forKey:@"tt46.emojiBar"];[vc showCategory:-1 animated:NO];
    [vc showCategory:4 animated:NO];[vc.view layoutIfNeeded];Check([vc.stack viewWithTag:620]&&[vc.stack viewWithTag:626]&&[vc.stack viewWithTag:627],@"61 bar swatches custom picker and reset reachable");Shot(vc,@"61-bar-color-swatches");
    UIButton *swatch=(id)[vc.stack viewWithTag:621];[swatch sendActionsForControlEvents:UIControlEventTouchUpInside];Check(JK61BarColor()!=nil,@"61 swatch action persists color");[vc barPicker:nil];
    NSDate *until=[NSDate dateWithTimeIntervalSinceNow:.8];while([until timeIntervalSinceNow]>0)[NSRunLoop.currentRunLoop runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:.02]];
    Check([vc.presentedViewController isKindOfClass:UIColorPickerViewController.class],@"61 public native color picker presented");UIColorPickerViewController *picker=(id)vc.presentedViewController;Check(!picker.supportsAlpha,@"61 custom bar color remains readable opaque");picker.selectedColor=UIColor.orangeColor;[vc colorPickerViewControllerDidSelectColor:picker];Check(JK61BarColor()!=nil,@"61 native picker delegate live-persisted");ShotView(self.window,@"61-native-color-picker");[vc dismissViewControllerAnimated:NO completion:nil];[vc barReset:nil];[vc showCategory:-1 animated:NO];
    NSLog(@"PASS 61 global explicit glass registry live tint/public blur/edge, debounce reload reset sync, no native mutation and independent floating switch");
}

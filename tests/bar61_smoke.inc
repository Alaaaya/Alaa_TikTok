{
    CGRect before=dock.frame;CGFloat radius=dock.layer.cornerRadius;NSArray *children=[dock.subviews copy];
    UILabel *unrelated=[UILabel new];unrelated.textColor=UIColor.purpleColor;UIColor *nativeInk=unrelated.textColor;
    UIView *nativePanel=[UIView new];nativePanel.backgroundColor=UIColor.redColor;
    [D() removeObjectForKey:@"tt61.barColor"];JK61BarApply(dock);Check(![dock viewWithTag:9369],@"61 default bottom color does nothing");
    UIColor *original=dock.backgroundColor;JK61SetBarColor([UIColor colorWithRed:.08 green:.32 blue:.26 alpha:1]);
    UIView *plate=[dock viewWithTag:9369];Check(plate&&!plate.userInteractionEnabled&&CGRectEqualToRect(before,dock.frame)&&dock.layer.cornerRadius==radius&&plate.layer.cornerRadius==radius,@"61 bar color preserves original shape/frame/navigation hit targets");
    UIColor *persisted=JK61BarColor();Check(persisted&&[D() arrayForKey:@"tt61.barColor"].count==3,@"61 RGB persisted and re-read without cached selection");
    Check([unrelated.textColor isEqual:nativeInk]&&[nativePanel.backgroundColor isEqual:UIColor.redColor]&&![JK61Bars() objectForKey:nativePanel],@"61 top panels/native surfaces not colored");
    for(NSUInteger i=0;i<5;i++){Check([((UIView *)tabs[i]).gestureRecognizers containsObject:tabGRs[i]],@"61 bottom-color preserves native action recognizers");}
    ShotView(self.window,@"61-colored-bottom-bar");
    JKRailClosed=YES;JKRailApply(self.window,NO);JK61SetBarColor(UIColor.whiteColor);Check(dock.hidden&&dock.alpha==0&&!dock.userInteractionEnabled,@"61 changing bottom color cannot reveal collapsed bar");
    JKRailClosed=NO;JKRailApply(self.window,NO);Check(!dock.hidden&&dock.alpha>0&&dock.userInteractionEnabled,@"61 expansion restores colored dock without action regression");
    JK61SetBarColor(nil);JK61BarApply(dock);Check(![dock viewWithTag:9369]&&!JK61BarColor()&&[dock.backgroundColor isEqual:original]&&CGRectEqualToRect(before,dock.frame),@"61 reset removes override and restores current theme rather than hardcoded background");
    dock.hidden=YES;dock.alpha=.21;dock.userInteractionEnabled=NO;JK61SetBarColor(UIColor.blueColor);Check(dock.hidden&&dock.alpha==.21&&!dock.userInteractionEnabled,@"61 color changes respect independently hidden/dim/noninteractive bar");JK61SetBarColor(nil);dock.hidden=NO;dock.alpha=1;dock.userInteractionEnabled=YES;
    JK61SetBarColor(UIColor.orangeColor);JK61BarRestore(dock);dock.backgroundColor=UIColor.cyanColor;JK61BarApply(dock);JK61SetBarColor(nil);Check([dock.backgroundColor isEqual:UIColor.cyanColor],@"61 reset after theme update keeps the current theme background");dock.backgroundColor=original;
    for(UIView *v in children)Check(v.superview==dock,@"61 bottom color never removes existing native/decor children");
    NSLog(@"PASS 61 bottom-bar color opt-in/persistence/theme reset/shape/native recognizers/collapse isolation");
}

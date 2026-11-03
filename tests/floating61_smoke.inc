// Production floating construction, drag, material and custom-art helpers.
{
    TT122Hub *hub=[TT122Hub new];hub.overlay=[[TT122Passthrough alloc] initWithFrame:self.window.bounds];[self.window addSubview:hub.overlay];
    [D() removeObjectForKey:@"tt61.feedFloating"];[D() setBool:YES forKey:@"tt46.emojiBar"];
    Check(TT61FloatingHidden(@"tt122.eye")&&TT61FloatingHidden(@"tt122.download"),@"61 OFF preserves 60 emoji behavior");
    [D() setBool:YES forKey:@"tt61.feedFloating"];
    Check([D() boolForKey:@"tt46.emojiBar"]&&!TT61FloatingHidden(@"tt122.eye")&&!TT61FloatingHidden(@"tt122.download"),@"61 floating and emoji independently enabled/persisted");
    [TTToolCache() removeAllObjects];
    for(NSInteger tag=1221;tag<=1222;tag++){[NSFileManager.defaultManager removeItemAtPath:TTToolPath(tag) error:nil];}
    UIButton *download=[hub floating:@"arrow.down" tag:1221],*eye=[hub floating:@"eye" tag:1222];[hub.overlay addSubview:download];[hub.overlay addSubview:eye];
    TT61RefreshMaterial(download,YES);TT61RefreshMaterial(eye,YES);[hub place:download y:210];[hub place:eye y:280];
    Check([hub.overlay hitTest:CGPointMake(200,100) withEvent:nil]==nil,@"61 overlay never steals unrelated native touches");
    Check(![download viewWithTag:1261].hidden&&[JK61Glasses() objectForKey:[download viewWithTag:1261]].tintColor==UIColor.clearColor,@"61 clear floating glass has no opaque plate");
    for(UIButton *b in @[download,eye])Check([b.subviews indexOfObject:b.imageView]>[b.subviews indexOfObject:[b viewWithTag:1261]],@"61 floating symbols are above glass at creation, not only after refresh");
    NSInteger taps=hub.tapCount;[eye sendActionsForControlEvents:UIControlEventTouchDown];[eye sendActionsForControlEvents:UIControlEventTouchUpInside];Check(hub.tapCount==taps+1,@"61 native tap remains actionable");
    JK60TestPan *pan=[JK60TestPan new];[eye addGestureRecognizer:pan];
    for(NSValue *point in @[[NSValue valueWithCGPoint:CGPointMake(-900,-900)],[NSValue valueWithCGPoint:CGPointMake(160,230)],[NSValue valueWithCGPoint:CGPointMake(1800,1800)]]){
        [eye sendActionsForControlEvents:UIControlEventTouchDown];pan.fixtureState=UIGestureRecognizerStateBegan;pan.fixtureDelta=CGPointZero;[hub pan:pan];pan.fixtureState=UIGestureRecognizerStateChanged;pan.fixtureDelta=point.CGPointValue;[hub pan:pan];pan.fixtureState=UIGestureRecognizerStateEnded;[hub pan:pan];
        taps=hub.tapCount;[eye sendActionsForControlEvents:UIControlEventTouchUpInside];Check(hub.tapCount==taps,@"61 drag cannot also tap");
        Check(CGRectContainsRect(UIEdgeInsetsInsetRect(hub.overlay.bounds,hub.overlay.safeAreaInsets),eye.frame),@"61 drag safe-area clamped all edges");
        [hub tapped:eye];Check(hub.tapCount==taps+1,@"61 sheet/emoji callbacks not poisoned by a prior floating drag");
    }
    NSDictionary *saved=[[D() dictionaryForKey:@"tt122.position.1222"] copy];
    UIButton *recreated=[hub floating:@"eye" tag:1222];[hub place:recreated y:0];Check(CGPointEqualToPoint(recreated.center,eye.center),@"61 saved main-feed drag restores after recreation");
    CGRect portrait=hub.overlay.frame;hub.overlay.frame=CGRectMake(0,0,portrait.size.height,portrait.size.width);[hub place:eye y:0];CGPoint expected=[hub clamp:CGPointMake([saved[@"x"] doubleValue]*hub.overlay.bounds.size.width,[saved[@"y"] doubleValue]*hub.overlay.bounds.size.height) view:eye];Check(CGPointEqualToPoint(expected,eye.center),@"61 rotation uses persisted normalized position");hub.overlay.frame=portrait;[hub place:eye y:0];
    // Real refresh-layout helper must notice rotation even after UIKit already
    // autoresized the overlay to the new window bounds.
    hub.downloadButton=download;hub.eyeButton=eye;
    hub.overlay.autoresizingMask=UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
    [hub layoutFloatingInWindow:self.window];CGRect windowPortrait=self.window.frame;
    self.window.frame=CGRectMake(windowPortrait.origin.x,windowPortrait.origin.y,windowPortrait.size.height,windowPortrait.size.width);
    hub.overlay.frame=self.window.bounds; // Model autoresizing before refresh.
    [hub layoutFloatingInWindow:self.window];
    CGPoint rotated=[hub clamp:CGPointMake([saved[@"x"] doubleValue]*hub.overlay.bounds.size.width,[saved[@"y"] doubleValue]*hub.overlay.bounds.size.height) view:eye];
    Check(CGSizeEqualToSize(hub.placedSize,self.window.bounds.size)&&hypot(eye.center.x-rotated.x,eye.center.y-rotated.y)<.01,@"61 real refresh layout repositions normalized drag after UIKit autoresizes overlay");
    self.window.frame=windowPortrait;hub.overlay.frame=self.window.bounds;[hub layoutFloatingInWindow:self.window];
    [eye sendActionsForControlEvents:UIControlEventTouchDown];pan.fixtureState=UIGestureRecognizerStateBegan;pan.fixtureDelta=CGPointMake(2,2);[hub pan:pan];pan.fixtureState=UIGestureRecognizerStateEnded;[hub pan:pan];Check(!hub.dragExceeded,@"61 sub8pt translation is not a drag");
    [eye sendActionsForControlEvents:UIControlEventTouchUpInside];
    [hub place:download y:210];eye.center=CGPointMake(40,280);ShotView(self.window,@"61-floating-main-feed");
    JKRailClosed=YES;JKRailApply(self.window,NO);JK60Tree(hub.overlay);Check(eye.hidden&&download.hidden&&![(JKAlaaToggle *)[self.window viewWithTag:9156] isHidden],@"61 full60 collapse hides floating buttons, keeps dots");ShotView(self.window,@"61-collapsed-only-dots");
    JKRailClosed=NO;JKRailApply(self.window,NO);Check(!eye.hidden&&!download.hidden,@"61 collapse expand restores floating state");
    TT50EyeActive=YES;TT50Tree(fd);Check(fr.hidden&&!eye.hidden,@"61 eye semantics preserve restore button outside feed chrome");TT50Restore();
    // Persist real transparent rectangular PNG. Red bar spans both outer edges;
    // transparent middle must show no glass, default symbol, border or shadow.
    UIGraphicsImageRendererFormat *fmt=[UIGraphicsImageRendererFormat defaultFormat];fmt.opaque=NO;fmt.scale=1;
    UIImage *art=[[[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(80,40) format:fmt] imageWithActions:^(UIGraphicsImageRendererContext *ctx){[UIColor.redColor setFill];UIRectFill(CGRectMake(0,0,80,10));[UIColor.blueColor setFill];UIRectFill(CGRectMake(0,30,80,10));}];
    [NSFileManager.defaultManager createDirectoryAtPath:TTToolPath(1221).stringByDeletingLastPathComponent withIntermediateDirectories:YES attributes:nil error:nil];
    [UIImagePNGRepresentation(art) writeToFile:TTToolPath(1221) atomically:YES];[TTToolCache() removeObjectForKey:@1221];TT61ButtonArt(download,@"arrow.down");[download layoutIfNeeded];
    Check([download viewWithTag:1261].hidden&&download.layer.borderWidth==0&&download.layer.shadowOpacity==0&&!download.clipsToBounds&&!download.layer.masksToBounds&&UIEdgeInsetsEqualToEdgeInsets(download.imageEdgeInsets,UIEdgeInsetsZero)&&download.imageView.contentMode==UIViewContentModeScaleAspectFit,@"61 full custom image suppresses every plate/rim/glyph and never circle clips or crops alpha");
    Check([download imageForState:UIControlStateNormal]==TTToolImage(1221)&&!download.adjustsImageWhenHighlighted,@"61 full original-color image is the only button art");
    UIImage *rendered=[[[UIGraphicsImageRenderer alloc] initWithSize:download.bounds.size format:fmt] imageWithActions:^(UIGraphicsImageRendererContext *ctx){[download.layer renderInContext:ctx.CGContext];}];
    unsigned char pixels[48*48*4]={0};CGColorSpaceRef space=CGColorSpaceCreateDeviceRGB();CGContextRef bitmap=CGBitmapContextCreate(pixels,48,48,8,48*4,space,kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);CGContextDrawImage(bitmap,CGRectMake(0,0,48,48),rendered.CGImage);CGContextRelease(bitmap);CGColorSpaceRelease(space);
    Check(pixels[(24*48+24)*4+3]==0,@"61 transparent PNG center has zero rendered alpha: no hidden default glyph/backplate");
    Check(pixels[(15*48+1)*4+3]>200||pixels[(32*48+1)*4+3]>200,@"61 rectangular artwork edge survives, not circular clipping");
    UIButton *foreign=[UIButton buttonWithType:UIButtonTypeSystem];[foreign setImage:[UIImage systemImageNamed:@"eye"] forState:UIControlStateNormal];UIImage *beforeForeign=[foreign imageForState:UIControlStateNormal];TT61ButtonArt(foreign,@"arrow.down");Check([foreign imageForState:UIControlStateNormal]==beforeForeign,@"61 image helper refuses non-floating identities");
    ShotView(self.window,@"61-floating-transparent-art");
    [TTToolCache() removeObjectForKey:@1221];Check(TTToolImage(1221)!=nil,@"61 custom art persists from disk across cache recreation");
    UIGraphicsImageRenderer *photoRenderer=[[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(48,48)];UIImage *photo=[photoRenderer imageWithActions:^(UIGraphicsImageRendererContext *c){[UIColor.orangeColor setFill];UIRectFill(CGRectMake(0,0,48,48));}];[UIImagePNGRepresentation(photo) writeToFile:TTToolPath(1221) atomically:YES];[TTToolCache() removeObjectForKey:@1221];TT61ButtonArt(download,@"arrow.down");[download layoutIfNeeded];Check(CGRectEqualToRect(download.imageView.frame,download.bounds),@"61 square photo fills entire visual bounds without icon insets");ShotView(self.window,@"61-floating-photo");
    JK61Set(@"edge",.9);JK61Set(@"tint",.8);Check([download viewWithTag:1261].hidden&&download.layer.borderWidth==0,@"61 custom art overrides global glass");
    [NSFileManager.defaultManager removeItemAtPath:TTToolPath(1221) error:nil];[TTToolCache() removeObjectForKey:@1221];TT61ButtonArt(download,@"arrow.down");Check(![download viewWithTag:1261].hidden&&[download imageForState:UIControlStateNormal]!=nil&&[download viewWithTag:1261].layer.borderWidth>0,@"61 reset restores current global glass, no residual image");
    JK61Reset();[D() setBool:NO forKey:@"tt61.feedFloating"];[D() setBool:NO forKey:@"tt46.emojiBar"];[hub.overlay removeFromSuperview];
    NSLog(@"PASS 61 real floating methods: independent toggles; passthrough; 8pt drag/persist/recreate/rotation; full collapse and eye; full custom PNG alpha/rectangle/photo/reset and emoji isolation");
}

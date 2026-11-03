{
    // Synthetic PUBLIC UIKit test data. Not real TikTok comment classes or a style mockup.
    Class fixtureClass=NSClassFromString(@"JK63FixtureCommentPanel");
    if(!fixtureClass){fixtureClass=objc_allocateClassPair(UIView.class,"JK63FixtureCommentPanel",0);objc_registerClassPair(fixtureClass);}
    UIView *panel=[[fixtureClass alloc] initWithFrame:CGRectMake(0,180,430,600)];
    panel.alpha=.82;[self.window addSubview:panel];
    UILabel *label=[UILabel new];label.text=@"SECRET_USERNAME_63 Follow";label.accessibilityLabel=@"SECRET_ACCESSIBILITY_63";label.accessibilityIdentifier=@"SECRET_IDENTIFIER_63";[panel addSubview:label];
    UITextField *input=[UITextField new];input.text=@"SECRET_PASSWORD_63";input.secureTextEntry=YES;[panel addSubview:input];
    UITextView *message=[UITextView new];message.text=@"SECRET_COMMENT_63";[panel addSubview:message];
    UIButton *like=[UIButton buttonWithType:UIButtonTypeSystem];[like setTitle:@"SECRET_TITLE_63" forState:UIControlStateNormal];[like addTarget:vc action:@selector(close) forControlEvents:UIControlEventTouchUpInside];like.selected=YES;[panel addSubview:like];
    UITapGestureRecognizer *tap=[[UITapGestureRecognizer alloc] initWithTarget:vc action:@selector(close)];[panel addGestureRecognizer:tap];
    CGRect savedFrame=panel.frame;NSUInteger subviews=panel.subviews.count;
    NSString *capture=JKHierarchyCapture();
    Check([capture containsString:@"JK63FixtureCommentPanel"] && [capture containsString:@"close@"] && [capture containsString:@"selected=1"] && [capture containsString:@"UITapGestureRecognizer"] && [capture containsString:@"secure=1"],@"63 actual UIKit capture records classes, controls, targets, gesture and secure flag");
    for(NSString *secret in @[@"SECRET_",@"Follow",@"SECRET_PASSWORD_63",@"SECRET_COMMENT_63"])Check(![capture containsString:secret],@"63 no text, titles, static control words or accessibility strings");
    Check([capture lengthOfBytesUsingEncoding:NSUTF8StringEncoding]<=JK_DUMP_BYTES,@"63 per-snapshot byte bound");
    Check(CGRectEqualToRect(savedFrame,panel.frame)&&panel.subviews.count==subviews&&fabs(panel.alpha-.82)<.001&&like.selected&&like.allTargets.count==1&&tap.enabled,@"63 read-only capture preserves frames, alpha, targets, gestures and states");
    JK63RememberBeforeSettings();[panel removeFromSuperview];
    NSString *folder=NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject;
    NSString *path=[folder stringByAppendingPathComponent:@"joker_comments63.txt"];
    [@"SECRET_OLD_FILE_63" writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil];
    JKAppendCapture(path);NSString *export=[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
    Check([export containsString:@"before Joker settings"]&&[export containsString:@"JK63FixtureCommentPanel"]&&![export containsString:@"SECRET_"],@"63 pre-settings snapshot retained, old file never included");
    Check(JK63BeforeSettings==nil && export.length<JK_DUMP_BYTES*2+700,@"63 cache consumed and combined export bounded");
    JKAppendCapture(path);export=[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
    Check(![export containsString:@"JK63FixtureCommentPanel"]&&[export containsString:@"No recent pre-settings"],@"63 no stale snapshots on next export");
    JK63BeforeSettings=@"SECRET_EXPIRED_63";JK63BeforeTime=NSProcessInfo.processInfo.systemUptime-121;JKAppendCapture(path);
    Check(![[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil] containsString:@"SECRET_EXPIRED_63"],@"63 snapshot expires after 120 seconds");
    // Stress broad and deep trees. Discovery queue and output are separately bounded.
    UIView *wide=[[fixtureClass alloc] init];for(NSUInteger i=0;i<2300;i++)[wide addSubview:[UIView new]];[self.window addSubview:wide];
    UIView *deep=wide;for(NSUInteger i=0;i<24;i++){UIView *child=[UIView new];[deep addSubview:child];deep=child;}
    NSString *stress=JKHierarchyCapture();Check(stress.length<=JK_DUMP_BYTES && [stress containsString:@"end nodes="],@"63 wide/deep fixture bounded and terminated");[wide removeFromSuperview];
    JK63BeforeSettings=@"fixture cache";JK63BeforeTime=0;JKAppendCapture(path);
    ShotView(self.window,@"63-diagnostic-settings-NOT-comments-style");
    NSLog(@"PASS 63 UIKit structure-only capture, canary privacy, no mutation, bounded stress, cache expiration and fresh-file guards");
}

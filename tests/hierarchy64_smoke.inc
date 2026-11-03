{
    // All synthetic PUBLIC UIKit structures, never asserted to be TikTok classes.
    UIWindow *testWindow=[[UIWindow alloc] initWithFrame:self.window.bounds];
    JK60TestModalRoot *root=[JK60TestModalRoot new];root.view=[[UIView alloc] initWithFrame:testWindow.bounds];
    testWindow.rootViewController=root;testWindow.hidden=NO;
    UIView *feed=[[UIView alloc] initWithFrame:root.view.bounds];[root.view addSubview:feed];
    UITableView *feedList=[[UITableView alloc] initWithFrame:feed.bounds];[feed addSubview:feedList];
    // Existing full-screen feed + composer must not count as a newly presented sheet.
    UITextField *feedInput=[[UITextField alloc] initWithFrame:CGRectMake(0,700,300,40)];[feed addSubview:feedInput];
    JK64Arm();JK64Scan(@"fixture_feed");Check(!JK64Last,@"64 baseline feed not captured");
    UIView *panel=[[UIView alloc] initWithFrame:CGRectMake(0,200,testWindow.bounds.size.width,testWindow.bounds.size.height-200)];
    UITableView *list=[[UITableView alloc] initWithFrame:CGRectMake(0,0,300,400)];[panel addSubview:list];
    UITextField *input=[[UITextField alloc] initWithFrame:CGRectMake(0,420,300,40)];input.text=@"SECRET_INPUT_64";input.secureTextEntry=YES;[panel addSubview:input];
    UILabel *label=[UILabel new];label.text=@"SECRET_COMMENT_64";label.accessibilityLabel=@"SECRET_ACCESSIBILITY_64";[panel addSubview:label];
    UIButton *button=[UIButton buttonWithType:UIButtonTypeSystem];[button setTitle:@"SECRET_TITLE_64" forState:UIControlStateNormal];[button addTarget:vc action:@selector(close) forControlEvents:UIControlEventTouchUpInside];[panel addSubview:button];
    UITapGestureRecognizer *gesture=[[UITapGestureRecognizer alloc] initWithTarget:vc action:@selector(close)];[panel addGestureRecognizer:gesture];
    CGRect original=panel.frame;[root.view addSubview:panel];
    JK64Scan(@"fixture_embedded_unknown");
    Check(JK64Last && [JK64Last containsString:@"new-generic-list-composer-sheet"] && [JK64Last containsString:@"secure=1"],@"64 embedded unknown class list+composer captured");
    Check(![JK64Last containsString:@"SECRET_"] && [JK64Last containsString:@"close@"] && [JK64Last containsString:@"UITapGestureRecognizer"],@"64 privacy redaction and public target metadata");
    Check(CGRectEqualToRect(original,panel.frame)&&gesture.enabled&&panel.gestureRecognizers.count==1&&button.allTargets.count==1,@"64 capture preserves native frames gestures and targets");
    NSString *saved=JK64Last;[panel removeFromSuperview];JK64Scan(@"fixture_feed-after-close");Check(JK64Last==saved,@"64 feed after close never replaces retained sheet");
    // Opening settings and exporting never scans or replaces the retained candidate.
    NSString *path=[NSTemporaryDirectory() stringByAppendingPathComponent:@"joker_comments64_fixture.txt"];
    [@"SECRET_OLD_FILE_64" writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil];
    JK64Export(path);NSString *export=[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
    Check([export containsString:@"fixture_embedded_unknown"]&&![export containsString:@"SECRET_"]&&[export containsString:@"retainedAgeSeconds="],@"64 later export is fresh redacted retained sheet with relative age");
    JK64Export(path);Check(JK64Last==saved,@"64 repeated share keeps only last sheet, no history");
    // Modal root, including a generic full-screen modal, independently discovered from public controller relations.
    JK64Arm();UIViewController *modal=[UIViewController new];modal.view=[[UIView alloc] initWithFrame:testWindow.bounds];
    root.fixturePresented=modal;[modal.view addSubview:panel];panel.frame=CGRectMake(0,0,testWindow.bounds.size.width,testWindow.bounds.size.height);
    [root.view addSubview:modal.view];
    // Fixture public presentation relation does not have presentingViewController, so use partial geometry here.
    panel.frame=CGRectMake(0,100,testWindow.bounds.size.width,testWindow.bounds.size.height-100);
    JK64Scan(@"fixture_modal");Check(JK64Last&&[JK64Last containsString:@"fixture_modal"],@"64 modal hierarchy discovery");
    root.fixturePresented=nil;[modal.view removeFromSuperview];[panel removeFromSuperview];
    JK64LastTime=JK64Now()-JK64_TTL-1;JK64Export(path);Check(!JK64Last,@"64 retained capture eligibility and RAM cleared on expiry");
    JK64Arm();JK64Deadline=JK64Now()-1;JK64Scan(@"expired");Check(!JK64Armed&&JK64Scans==0,@"64 short arm session expires without scan");
    JK64Arm();for(NSUInteger i=0;i<40;i++)JK64Scan(@"cap");Check(JK64Scans==JK64_SCAN_BUDGET&&!JK64Active(),@"64 finite total scan cap");
    JK64Arm();[root.view addSubview:panel];
    for(NSUInteger i=0;i<2200;i++)[panel addSubview:[UIView new]];
    JK64Scan(@"fixture_wide");
    Check(JK64Last.length<=JK_DUMP_BYTES&&[JK64Last containsString:@"discoveryCapped=1"],@"64 independent discovery/output caps reported");
    JK64Stop();[panel removeFromSuperview];
    // Reused hidden UIKit sheets are not in the visible identity baseline.
    while(panel.subviews.count>5)[panel.subviews.lastObject removeFromSuperview];
    panel.hidden=YES;[root.view addSubview:panel];JK64Arm();panel.hidden=NO;
    JK64Scan(@"fixture_hidden_reuse");
    Check(JK64Last&&[JK64Last containsString:@"fixture_hidden_reuse"],@"64 hidden reused sheet captured after visible transition");
    [panel removeFromSuperview];JK64Arm();
    UIView *strip=[[UIView alloc] initWithFrame:CGRectMake(0,testWindow.bounds.size.height-56,testWindow.bounds.size.width,56)];
    [strip addSubview:list];[strip addSubview:input];[root.view addSubview:strip];
    JK64Scan(@"fixture_small_strip");Check(!JK64Last,@"64 small feed emoji strip never qualifies");
    [strip removeFromSuperview];[panel addSubview:list];[panel addSubview:input];
    JK64Stop();Check(!JK64Active(),@"64 observer idle after stop");
    // Restore baseline fixtures and clean memory before 62 gesture/drag/reuse suites.
    testWindow.hidden=YES;testWindow.rootViewController=nil;JK64Last=nil;JK64Best=0;JK64LastTime=0;
    NSLog(@"PASS 64 synthetic embedded unknown/modal tree, baseline/feed retention, privacy, read-only gestures, expiry and hard scan/node/byte caps");
}

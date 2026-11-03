static void JK64AsyncFixture(UIWindow *original,void (^done)(void)){
    UIWindow *w=[[UIWindow alloc] initWithFrame:original.bounds];
    UIViewController *root=[UIViewController new];w.rootViewController=root;[w makeKeyAndVisible];
    [root loadViewIfNeeded];JK64Arm();
    UIViewController *sheet=[UIViewController new];sheet.modalPresentationStyle=UIModalPresentationFullScreen;
    [sheet loadViewIfNeeded];sheet.view.frame=w.bounds;
    UITableView *list=[[UITableView alloc] initWithFrame:CGRectMake(0,40,w.bounds.size.width,400)];[sheet.view addSubview:list];
    UITextField *input=[[UITextField alloc] initWithFrame:CGRectMake(0,500,300,44)];input.text=@"SECRET_ASYNC_64";[sheet.view addSubview:input];
    // Test the exact production scheduling functions; Logos wiring checked separately.
    [root presentViewController:sheet animated:NO completion:^{
        for(NSUInteger i=0;i<100;i++)JK64Event(@"fixture-public-modal");
        Check(JK64Events==1&&JK64Pending,@"64 lifecycle burst coalesced without frame polling");
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(1.1*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
            Check(JK64Last&&[JK64Last containsString:@"fixture-public-modal"]&&![JK64Last containsString:@"SECRET_"],@"64 actual public full-screen modal relation and finite main-queue capture");
            Check(JK64Scans==2,@"64 one settle snapshot plus one finite retry");
            NSString *saved=JK64Last;[root dismissViewControllerAnimated:NO completion:^{
                JK64Event(@"fixture-feed-dismiss");
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(1.1*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
                    Check(JK64Last==saved,@"64 real modal close/feed event never overwrites capture");
                    JK64Stop();JK64Last=nil;JK64LastTime=0;JK64Best=0;
                    w.hidden=YES;w.rootViewController=nil;[original makeKeyAndVisible];
                    NSLog(@"PASS 64 actual public UIKit modal and asynchronous coalesced two-shot scheduling; no polling or native event interception");
                    done();
                });
            }];
        });
    }];
}

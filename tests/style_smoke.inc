// 53 Instagram interface style smoke (fixtures are TEST DATA, not real TikTok views).
{
    static NSMutableArray *keep;if(!keep)keep=[NSMutableArray new];
    Class tcls=NSClassFromString(@"JKTestTarget");
    if(!tcls){tcls=objc_allocateClassPair(NSObject.class,"JKTestTarget",0);
        IMP imp=imp_implementationWithBlock(^(id me){void(^blk)(void)=objc_getAssociatedObject(me,"blk");if(blk)blk();});
        class_addMethod(tcls,NSSelectorFromString(@"fire"),imp,"v@:");objc_registerClassPair(tcls);}
    void(^JKTestWire)(UIControl *,void(^)(void))=^(UIControl *c,void(^blk)(void)){id t=[tcls new];objc_setAssociatedObject(t,"blk",blk,OBJC_ASSOCIATION_COPY_NONATOMIC);[keep addObject:t];[c addTarget:t action:NSSelectorFromString(@"fire") forControlEvents:UIControlEventTouchUpInside];};
    Class stackClass=NSClassFromString(@"TTKFeedRightInteractionStackView");
    if(!stackClass){stackClass=objc_allocateClassPair(UIView.class,"TTKFeedRightInteractionStackView",0);objc_registerClassPair(stackClass);}
    Class btnClass=NSClassFromString(@"AWEFeedVideoButton");
    if(!btnClass){btnClass=objc_allocateClassPair(UIButton.class,"AWEFeedVideoButton",0);objc_registerClassPair(btnClass);}
    CGFloat W=self.window.bounds.size.width,H=self.window.bounds.size.height;
    UIView *feedRoot=[[UIView alloc] initWithFrame:self.window.bounds];
    Class feedCls=NSClassFromString(@"TTKFeedFixtureView");
    if(!feedCls){feedCls=objc_allocateClassPair(UIView.class,"TTKFeedFixtureView",0);objc_registerClassPair(feedCls);}
    feedRoot=[[feedCls alloc] initWithFrame:self.window.bounds];
    feedRoot.backgroundColor=[UIColor colorWithRed:.45 green:.62 blue:.78 alpha:1];
    [self.window addSubview:feedRoot];
    UIView *rail=[[stackClass alloc] initWithFrame:CGRectMake(W-70,H*.42,60,330)];[feedRoot addSubview:rail];
    NSArray *ids=@[@"digg",@"comment",@"collect",@"share"];NSArray *counts=@[@"29.4K",@"163",@"87",@"19K"];
    NSMutableArray<UIButton *> *natives=[NSMutableArray new];NSMutableArray<NSNumber *> *fired=[NSMutableArray new];
    for(NSUInteger i=0;i<4;i++) {
        UIButton *b=[btnClass buttonWithType:UIButtonTypeCustom];b.frame=CGRectMake(0,i*80,60,72);
        b.accessibilityIdentifier=ids[i];
        UILabel *l=[[UILabel alloc] initWithFrame:CGRectMake(0,50,60,18)];l.text=counts[i];l.textColor=UIColor.whiteColor;l.font=[UIFont systemFontOfSize:12];l.textAlignment=NSTextAlignmentCenter;[b addSubview:l];
        UIImageView *iv=[[UIImageView alloc] initWithImage:[UIImage systemImageNamed:@"heart.fill"]];iv.frame=CGRectMake(14,8,32,32);iv.tintColor=UIColor.whiteColor;[b addSubview:iv];
        NSUInteger idx=i;b.tag=500+i;
        JKTestWire(b,^{[fired addObject:@(idx)];});
        [rail addSubview:b];[natives addObject:b];
    }
    // Unidentified control and an identified one without a native touch target: must stay native.
    UIButton *mystery=[btnClass buttonWithType:UIButtonTypeCustom];mystery.frame=CGRectMake(0,-80,60,72);[rail addSubview:mystery];
    JKTestWire(mystery,^{});
    UIButton *noTarget=[btnClass buttonWithType:UIButtonTypeCustom];noTarget.frame=CGRectMake(0,-160,60,72);noTarget.accessibilityIdentifier=@"digg";[rail addSubview:noTarget];
    UILabel *banner=[[UILabel alloc] initWithFrame:CGRectMake(10,70,W-20,22)];banner.text=@"TEST DATA - simulator fixture, not real TikTok";banner.textColor=UIColor.whiteColor;banner.font=[UIFont boldSystemFontOfSize:11];banner.textAlignment=NSTextAlignmentLeft;[feedRoot addSubview:banner];
    [D() setInteger:0 forKey:JKStyleKey];
    JKStyleApplyAll();
    Check([rail.subviews filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(id v,id b){return [v isKindOfClass:JKStyleItem.class];}]].count==0 && natives[0].alpha==1,@"53 default style leaves native UI untouched");
    ShotView(self.window,@"52-native-default");
    [D() setInteger:1 forKey:JKStyleKey];
    JKStyleApplyAll();
    NSArray *items=[rail.subviews filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(id v,id b){return [v isKindOfClass:JKStyleItem.class];}]];
    Check(items.count==4,@"53 four recognized actions replaced");
    Check(JKItemOf(mystery)==nil && mystery.alpha==1 && JKItemOf(noTarget)==nil && noTarget.alpha==1,@"53 unidentified or untargeted controls stay native");
    for(NSUInteger i=0;i<4;i++){
        JKStyleItem *it=JKItemOf(natives[i]);
        Check(it && natives[i].alpha==0 && JKStyleAlpha(natives[i],1)==0,@"53 native hidden synchronously");
        Check([it.countLabel.text isEqual:counts[i]],@"53 counts read from native labels");
        Check(CGRectEqualToRect(it.frame,natives[i].frame) && it.semanticContentAttribute==UISemanticContentAttributeForceLeftToRight,@"53 aligned LTR even under RTL locale");
    }
    Check(JKItemOf(natives[0]).kind==JKKindLike && JKItemOf(natives[1]).kind==JKKindComment && JKItemOf(natives[2]).kind==JKKindSave && JKItemOf(natives[3]).kind==JKKindShare,@"53 kinds mapped");
    [JKItemOf(natives[1]) sendActionsForControlEvents:UIControlEventTouchUpInside];
    [JKItemOf(natives[3]) sendActionsForControlEvents:UIControlEventTouchUpInside];
    Check(fired.count==2 && [fired[0] isEqual:@1] && [fired[1] isEqual:@3],@"53 taps forward to exactly the matching native control");
    // Count refresh and reuse without duplicates.
    for(UIView *v in natives[0].subviews)if([v isKindOfClass:UILabel.class])((UILabel *)v).text=@"29.5K";
    natives[0].selected=YES;JKStyleSync(natives[0]);JKStyleSync(natives[0]);JKStyleSync(natives[2]);
    Check([JKItemOf(natives[0]).countLabel.text isEqual:@"29.5K"],@"53 count refreshed on reuse");
    Check([rail.subviews filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(id v,id b){return [v isKindOfClass:JKStyleItem.class];}]].count==4,@"53 no duplicate overlays");
    natives[0].selected=NO;JKStyleSync(natives[0]);
    // Pass-through: blank rail area is not captured by overlay items.
    UIView *hit=[feedRoot hitTest:[feedRoot convertPoint:CGPointMake(W-100,H*.5) fromView:self.window] withEvent:nil];
    Check(hit==feedRoot,@"53 blank feed area passes through to video");
    // Eye hide hides overlay too, then restores.
    TT50EyeActive=YES;TT50Tree(feedRoot);
    JKStyleItem *eyeItem=JKItemOf(natives[0]);
    Check(eyeItem.alpha==0 && eyeItem.hidden,@"53 eye hide covers overlay");
    TT50Restore();
    Check(eyeItem.alpha==1 && !eyeItem.hidden && natives[0].alpha==0,@"53 eye restore keeps overlay, native still replaced");
    ShotView(self.window,@"52-instagram-style");
    // Unavailable styles cannot be selected.
    UIStackView *pickerHost=[UIStackView new];JKAddStylePicker(pickerHost);
    UISegmentedControl *seg=(UISegmentedControl *)pickerHost.arrangedSubviews[1];
    Check(seg.numberOfSegments==4 && seg.selectedSegmentIndex==1 && [[seg titleForSegmentAtIndex:1] isEqualToString:@"Instagram"],@"53 picker shows four styles, TikTok + Instagram selectable");
    seg.selectedSegmentIndex=2;[seg sendActionsForControlEvents:UIControlEventValueChanged];
    Check(JKStyle()==1 && seg.selectedSegmentIndex==1,@"53 unavailable Facebook rejected");
    Check(JKStyle()==1,@"53 Facebook rejected leaves style");
    seg.selectedSegmentIndex=1;[seg sendActionsForControlEvents:UIControlEventValueChanged];
    Check(JKStyle()==1 && seg.selectedSegmentIndex==1,@"53 Instagram selectable");
    {
        UITabBar *dumpBar=[[UITabBar alloc] initWithFrame:CGRectMake(0,0,300,50)];
        UILabel *secret=[UILabel new];secret.text=@"SECRETUSERCAPTION";secret.accessibilityLabel=@"SECRETAX";
        UILabel *known=[UILabel new];known.text=@"Follow";
        [dumpBar addSubview:secret];[dumpBar addSubview:known];[self.window addSubview:dumpBar];
        NSString *cap=JKHierarchyCapture();
        Check([cap containsString:@"UITabBar"] && [cap containsString:@"<text len=17>"] && [cap containsString:@"\"Follow\""],@"53 capture lists classes, redacts text, keeps control words");
        Check(![cap containsString:@"SECRETUSERCAPTION"] && ![cap containsString:@"SECRETAX"],@"53 capture never contains user text");
        Check(cap.length<JK_DUMP_BYTES+4000,@"53 capture bounded");
        [dumpBar removeFromSuperview];
    }
    seg.selectedSegmentIndex=3;[seg sendActionsForControlEvents:UIControlEventValueChanged];
    Check(JKStyle()==1,@"53 unavailable Joker rejected");
    seg.selectedSegmentIndex=0;[seg sendActionsForControlEvents:UIControlEventValueChanged];
    Check(JKStyle()==0 && JKItemOf(natives[0])==nil && natives[0].alpha==1 && natives[1].alpha==1 && natives[3].alpha==1,@"53 returning to TikTok restores originals");
    Check([rail.subviews filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(id v,id b){return [v isKindOfClass:JKStyleItem.class];}]].count==0,@"53 no leftover overlays");
    // Non-feed ancestor: no replacement.
    [D() setInteger:1 forKey:JKStyleKey];
    UIView *profile=[UIView new];Class profCls=NSClassFromString(@"TTKProfileFixtureView");if(!profCls){profCls=objc_allocateClassPair(UIView.class,"TTKProfileFixtureView",0);objc_registerClassPair(profCls);}
    profile=[[profCls alloc] initWithFrame:self.window.bounds];[self.window addSubview:profile];
    UIButton *pb=[btnClass buttonWithType:UIButtonTypeCustom];pb.frame=CGRectMake(W-70,H*.5,60,72);pb.accessibilityIdentifier=@"digg";
    JKTestWire(pb,^{});[profile addSubview:pb];
    JKStyleSync(pb);Check(JKItemOf(pb)==nil && pb.alpha==1,@"53 non-feed screens untouched");
    [profile removeFromSuperview];
    [D() setInteger:0 forKey:JKStyleKey];JKStyleApplyAll();
    [feedRoot removeFromSuperview];

    // 54: in-place dock / progress / username restyle, fully restored.
    {
        [D() setInteger:1 forKey:JKStyleKey];
        UIView *tb=[[NSClassFromString(@"TTKTabBarBlurView")?:UIView.class alloc] initWithFrame:CGRectMake(0,0,430,83)];
        Class pc=objc_allocateClassPair(UIView.class,"AWEFeedPlayerBottomProgressBar",0);if(pc)objc_registerClassPair(pc);
        UIView *bar=[[NSClassFromString(@"AWEFeedPlayerBottomProgressBar") alloc] initWithFrame:CGRectMake(12,846,406,2)];
        UIView *fill=[[UIView alloc] initWithFrame:CGRectMake(0,0,100,2)];fill.backgroundColor=UIColor.redColor;[bar addSubview:fill];
        [self.window addSubview:bar];[self.window addSubview:tb];
        JKInstaApplyAll();
        Check(fill.backgroundColor==UIColor.whiteColor,@"54 progress fill white");
        if([NSStringFromClass(tb.class) isEqualToString:@"TTKTabBarBlurView"])Check(tb.layer.cornerRadius>20 && tb.frame.origin.x==18,@"54 dock floating pill");
        [D() setInteger:0 forKey:JKStyleKey];JKInstaApplyAll();
        Check(CGColorEqualToColor(fill.backgroundColor.CGColor,UIColor.redColor.CGColor),@"54 progress restored");
        Check(CGRectEqualToRect(tb.frame,CGRectMake(0,0,430,83)) && tb.layer.cornerRadius==0,@"54 dock restored");
        [bar removeFromSuperview];[tb removeFromSuperview];
        NSLog(@"PASS 54 in-place dock/progress restyle and restore");
    }
    NSLog(@"PASS 53 Instagram interface style fixtures: forwarding, counts, reuse, eye, picker gating, restoration");
}

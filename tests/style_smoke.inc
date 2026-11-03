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
    seg.selectedSegmentIndex=2;[seg sendActionsForControlEvents:UIControlEventValueChanged];
    Check(JKStyle()==1,@"53 unavailable Facebook rejected");
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
    // 56: Alaa (فيض) style, style index 3.
    {
        [D() setInteger:3 forKey:JKStyleKey];
        Check(JKStyleOn() && JKAlaa(),@"56 Alaa style selectable and on");
        UIView *tb=[[NSClassFromString(@"TTKTabBarBlurView")?:UIView.class alloc] initWithFrame:CGRectMake(0,0,430,83)];
        [self.window addSubview:tb];JKInstaApplyAll();
        if([NSStringFromClass(tb.class) isEqualToString:@"TTKTabBarBlurView"])Check(tb.layer.cornerRadius>20 && tb.layer.borderWidth>0 && tb.backgroundColor!=nil,@"56 dock glass pill");
        JKStyleItem *it=[[JKStyleItem alloc] initWithFrame:CGRectMake(0,0,60,72)];it.kind=JKKindSave;
        UIButton *nb=[UIButton buttonWithType:UIButtonTypeCustom];nb.selected=YES;JKTestWire(nb,^{});it.native=nb;[it refresh];
        CGFloat r,g,bl,a;[it.glyph.tintColor getRed:&r green:&g blue:&bl alpha:&a];
        Check(r>.99 && g>.7 && g<.72 && bl<.3,@"56 saved glyph amber FFB547");
        it.kind=JKKindLike;[it refresh];[it.glyph.tintColor getRed:&r green:&g blue:&bl alpha:&a];
        Check(r>.99 && g>.29 && g<.31,@"56 liked glyph heart FF4D6A");
        UIStackView *ph=[UIStackView new];JKAddStylePicker(ph);
        UISegmentedControl *sg=(UISegmentedControl *)ph.arrangedSubviews[1];
        Check(sg.selectedSegmentIndex==3 && [[sg titleForSegmentAtIndex:3] isEqualToString:@"Alaa"],@"56 picker shows Alaa selected");
        // Native-feed fixture from the observed hierarchy (TEST DATA): rail, description, progress, dock.
        UIView *fd=[[feedCls alloc] initWithFrame:self.window.bounds];[self.window addSubview:fd];
        CAGradientLayer *gl=[CAGradientLayer layer];gl.frame=fd.bounds;
        gl.colors=@[(id)[UIColor colorWithRed:.04 green:.07 blue:.2 alpha:1].CGColor,(id)[UIColor colorWithRed:.85 green:.35 blue:.2 alpha:1].CGColor,(id)[UIColor colorWithRed:.25 green:.05 blue:.2 alpha:1].CGColor];
        [fd.layer addSublayer:gl];
        UIView *fr=[[stackClass alloc] initWithFrame:CGRectMake(W-70,H-122-420,60,420)];[fd addSubview:fr];
        NSMutableArray<UIButton *> *fn=[NSMutableArray new];NSMutableArray<NSNumber *> *ff=[NSMutableArray new];
        for(NSUInteger i=0;i<4;i++){
            UIButton *b=[btnClass buttonWithType:UIButtonTypeCustom];b.frame=CGRectMake(0,i*80,60,72);b.accessibilityIdentifier=@[@"digg",@"comment",@"collect",@"share"][i];
            UILabel *cl=[[UILabel alloc] initWithFrame:CGRectMake(0,50,60,18)];cl.text=@[@"29.4K",@"163",@"87",@"19K"][i];cl.textColor=UIColor.whiteColor;cl.font=[UIFont systemFontOfSize:12];cl.textAlignment=NSTextAlignmentCenter;[b addSubview:cl];
            NSUInteger ix=i;JKTestWire(b,^{[ff addObject:@(ix)];});[fr addSubview:b];[fn addObject:b];
        }
        Class mvc=NSClassFromString(@"AWEPlayInteractionMusicView");if(!mvc){mvc=objc_allocateClassPair(UIView.class,"AWEPlayInteractionMusicView",0);objc_registerClassPair(mvc);}
        UIView *mv=[[mvc alloc] initWithFrame:CGRectMake(10,326,40,40)];mv.backgroundColor=[UIColor colorWithRed:.1 green:.6 blue:.6 alpha:1];mv.layer.cornerRadius=20;[fr addSubview:mv];
        Class dc=NSClassFromString(@"AWEPlayInteractionDescriptionLabel");if(!dc){dc=objc_allocateClassPair(UILabel.class,"AWEPlayInteractionDescriptionLabel",0);objc_registerClassPair(dc);}
        UILabel *dl=[[dc alloc] initWithFrame:CGRectMake(12,H-200,318,42)];dl.numberOfLines=2;dl.textColor=UIColor.whiteColor;dl.font=[UIFont systemFontOfSize:14];
        dl.attributedText=[[NSAttributedString alloc] initWithString:@"The sun goes down and takes the day with it #sunset #desert #calm" attributes:@{NSForegroundColorAttributeName:UIColor.whiteColor,NSFontAttributeName:[UIFont systemFontOfSize:14]}];
        [fd addSubview:dl];
        UIView *pbar=[[NSClassFromString(@"AWEFeedPlayerBottomProgressBar") alloc] initWithFrame:CGRectMake(12,H-86,406,2)];
        UIView *pf=[[UIView alloc] initWithFrame:CGRectMake(0,0,300,2)];pf.backgroundColor=UIColor.redColor;[pbar addSubview:pf];[fd addSubview:pbar];
        Class pc2=NSClassFromString(@"AWETabBarPlusButton");if(!pc2){pc2=objc_allocateClassPair(UIButton.class,"AWETabBarPlusButton",0);objc_registerClassPair(pc2);}
                Class tbc=NSClassFromString(@"TTKTabBar");if(!tbc){tbc=objc_allocateClassPair(UITabBar.class,"TTKTabBar",0);objc_registerClassPair(tbc);}
        Class bbc=NSClassFromString(@"TTKTabBarBlurView");if(!bbc){bbc=objc_allocateClassPair(UIView.class,"TTKTabBarBlurView",0);objc_registerClassPair(bbc);}
        UITabBar *tabbar=[[tbc alloc] initWithFrame:CGRectMake(0,H-83,430,83)];[fd addSubview:tabbar];
        UIView *dock=[[bbc alloc] initWithFrame:tabbar.bounds];dock.backgroundColor=[UIColor colorWithWhite:0 alpha:.7];[tabbar insertSubview:dock atIndex:0];
        NSArray *tn=@[@"الرئيسية",@"اكتشف",@"أنا",@"الإعدادات"],*ti=@[@"house",@"magnifyingglass",@"person",@"slider.horizontal.3"];NSMutableArray *tis=[NSMutableArray new];
        for(NSUInteger i=0;i<4;i++)[tis addObject:[[UITabBarItem alloc] initWithTitle:tn[i] image:[UIImage systemImageNamed:ti[i]] tag:i]];
        tabbar.items=tis;tabbar.selectedItem=tis[0];tabbar.backgroundImage=[UIImage new];tabbar.shadowImage=[UIImage new];tabbar.tintColor=UIColor.whiteColor;tabbar.unselectedItemTintColor=[UIColor colorWithWhite:1 alpha:.75];
        UIView *plusB=[[pc2 alloc] initWithFrame:CGRectMake((W-86)/2,0,86,49)];[tabbar addSubview:plusB];
        UIColor *tintBefore=tabbar.tintColor;
        Class tcc=NSClassFromString(@"TikTokFeedTabControl");if(!tcc){tcc=objc_allocateClassPair(UIView.class,"TikTokFeedTabControl",0);objc_registerClassPair(tcc);}
        UIView *tc=[[tcc alloc] initWithFrame:CGRectMake(54,59,323,44)];[fd addSubview:tc];
        UIButton *t1=[UIButton buttonWithType:UIButtonTypeCustom];t1.frame=CGRectMake(70,0,80,44);[t1 setTitle:@"لك" forState:UIControlStateNormal];t1.selected=YES;[tc addSubview:t1];
        UIButton *t2=[UIButton buttonWithType:UIButtonTypeCustom];t2.frame=CGRectMake(160,0,80,44);[t2 setTitle:@"أتابعهم" forState:UIControlStateNormal];[tc addSubview:t2];
        Class cic=NSClassFromString(@"TTKFeedTabCornerItemView");if(!cic){cic=objc_allocateClassPair(UIView.class,"TTKFeedTabCornerItemView",0);objc_registerClassPair(cic);}
        UIView *c1=[[cic alloc] initWithFrame:CGRectMake(8,61,40,40)],*c2=[[cic alloc] initWithFrame:CGRectMake(382,61,40,40)];[fd addSubview:c1];[fd addSubview:c2];
        Class fpc=NSClassFromString(@"AWEPlayInteractionFollowPromptView");if(!fpc){fpc=objc_allocateClassPair(UIView.class,"AWEPlayInteractionFollowPromptView",0);objc_registerClassPair(fpc);}
        UIView *fp=[[fpc alloc] initWithFrame:CGRectMake(W-70,H-122-330-70,60,40)];[fd addSubview:fp];
        Class auc=NSClassFromString(@"AWEPlayInteractionAuthorView");if(!auc){auc=objc_allocateClassPair(UIView.class,"AWEPlayInteractionAuthorView",0);objc_registerClassPair(auc);}
        UIView *au=[[auc alloc] initWithFrame:CGRectMake(12,H-232,318,22)];[fd addSubview:au];
        UIButton *unb=[btnClass buttonWithType:UIButtonTypeCustom];unb.frame=CGRectMake(0,0,90,22);[au addSubview:unb];
        Class nlc=NSClassFromString(@"AWEUserNameLabel");if(!nlc){nlc=objc_allocateClassPair(UIView.class,"AWEUserNameLabel",0);objc_registerClassPair(nlc);}
        UIView *nlv=[[nlc alloc] initWithFrame:CGRectMake(0,0,90,22)];[unb addSubview:nlv];
        UILabel *nll=[[UILabel alloc] initWithFrame:nlv.bounds];nll.text=@"ليالي الكثبان";nll.textColor=UIColor.whiteColor;nll.font=[UIFont systemFontOfSize:15 weight:UIFontWeightSemibold];[nlv addSubview:nll];
        Class avc=NSClassFromString(@"AWEPlayInteractionUserAvatarView");if(!avc){avc=objc_allocateClassPair(UIView.class,"AWEPlayInteractionUserAvatarView",0);objc_registerClassPair(avc);}
        UIView *av=[[avc alloc] initWithFrame:CGRectMake(W-70,H-232-100,60,88)];[fd addSubview:av];
        __block int avHits=0;UIButton *avb=[btnClass buttonWithType:UIButtonTypeCustom];avb.frame=CGRectMake(6,6,44,44);{UIGraphicsImageRenderer *ir=[[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(44,44)];
          UIImage *im=[ir imageWithActions:^(UIGraphicsImageRendererContext *c){CGColorSpaceRef cs=CGColorSpaceCreateDeviceRGB();NSArray *cols=@[(id)[UIColor colorWithRed:.9 green:.5 blue:.2 alpha:1].CGColor,(id)[UIColor colorWithRed:.3 green:.2 blue:.6 alpha:1].CGColor];CGGradientRef g=CGGradientCreateWithColors(cs,(CFArrayRef)cols,NULL);CGContextDrawLinearGradient(c.CGContext,g,CGPointZero,CGPointMake(44,44),0);CGGradientRelease(g);CGColorSpaceRelease(cs);
              NSDictionary *at=@{NSFontAttributeName:[UIFont boldSystemFontOfSize:20],NSForegroundColorAttributeName:UIColor.whiteColor};[@"ل" drawAtPoint:CGPointMake(14,9) withAttributes:at];}];
          [avb setImage:im forState:UIControlStateNormal];}[av addSubview:avb];JKTestWire(avb,^{avHits++;});
        JKStyleApplyAll();
        Check([plusB viewWithTag:9157]!=nil && ![plusB viewWithTag:9157].userInteractionEnabled,@"56 amber plus overlay, inert");
        {CGFloat r1,g1,b1,a1;[tabbar.tintColor getRed:&r1 green:&g1 blue:&b1 alpha:&a1];Check(r1>.99&&g1>.7&&g1<.72,@"56 dock tint amber");}
        Check([tc viewWithTag:9158]!=nil && CGRectGetMidX([tc viewWithTag:9158].frame)==CGRectGetMidX(t1.frame),@"56 tab underline under selected native tab");
        t1.selected=NO;JKStyleApplyAll();Check([tc viewWithTag:9158]==nil,@"56 no selected tab -> no underline (fail closed)");t1.selected=YES;JKStyleApplyAll();
        Check(c1.layer.cornerRadius==20 && c2.layer.cornerRadius==20,@"56 corner items glass circles");
        Check([fp viewWithTag:9159]!=nil && ![fp viewWithTag:9159].userInteractionEnabled,@"56 follow chip presentational");
        {CGRect ar=av.frame;CGRect ur=[au convertRect:au.bounds toView:self.window];CGRect nr2=[nlv convertRect:nlv.bounds toView:self.window];CGRect fr2=fp.frame;
         CGFloat gy=CGRectGetMidY([avb convertRect:avb.bounds toView:self.window]),gx=CGRectGetMidX([avb convertRect:avb.bounds toView:self.window]);
         Check(av.superview==self.window && fabs(gy-CGRectGetMidY(ur))<2 && fabs(gx-33)<2,@"56 who row: round avatar left, same row as username");
         Check(CGRectGetMinX(nr2)>=gx+17 && CGRectGetMinX(nr2)-gx<40,@"56 username right of avatar");
         Check(fp.superview==self.window && CGRectGetMinX(fr2)>=CGRectGetMaxX(nr2) && fabs(CGRectGetMidY(fr2)-CGRectGetMidY(ur))<2,@"56 follow chip right of username same row");
         Check(avb.layer.cornerRadius==22 && avb.clipsToBounds,@"56 avatar image round clipped");(void)ar;}
        [avb sendActionsForControlEvents:UIControlEventTouchUpInside];Check(avHits==1 && avb.superview==av,@"56 avatar native action preserved");
        {CGRect dr=[dock convertRect:dock.bounds toView:self.window];CGRect pr=[plusB convertRect:plusB.bounds toView:self.window];
         Check(fabs(dr.origin.x-16)<.5 && fabs(dr.size.width-(W-32))<.5 && fabs(dr.size.height-62)<.5 && fabs(H-CGRectGetMaxY(dr)-12)<.5 && dock.layer.cornerRadius==31,@"56 floating dock x16 bottom12 h62 r31 with gap below");
         Check(pr.origin.y>=dr.origin.y-1 && CGRectGetMaxY(pr)<=CGRectGetMaxY(dr)+1,@"56 tab items inside pill");
         UIImageView *ac=(UIImageView *)[fr viewWithTag:9161];UIImageView *dsc=(UIImageView *)[fr viewWithTag:9162];
         Check(ac && ac.image && !ac.userInteractionEnabled && fabs(ac.frame.size.width-46)<.5 && ac.layer.cornerRadius==23 && ac.clipsToBounds,@"56 rail avatar copy 46 round inert");
         Check(dsc && dsc.image && !dsc.userInteractionEnabled && dsc.frame.size.width==42 && [dsc.layer animationForKey:@"jkspin"],@"56 music disc copy 42 spinning inert");
         UIView *hit=[fr hitTest:[fr convertPoint:CGPointMake(CGRectGetMidX(ac.frame),CGRectGetMidY(ac.frame)) toView:fr] withEvent:nil];Check(hit!=ac,@"56 avatar copy never takes touches");}
        JKStyleApplyAll();Check(av.superview==self.window,@"56 avatar reuse stable");
        UIView *tgl=[fr viewWithTag:9156];
        Check(tgl!=nil && CGRectGetMaxY(tgl.frame)>=fr.bounds.size.height-1 && JKRailClosed,@"56 rail toggle present and closed by default");
        for(UIView *v in fr.subviews)if([v isKindOfClass:JKStyleItem.class])Check(v.alpha==0 && !v.userInteractionEnabled,@"56 collapsed items hidden and inert");
        ShotView(self.window,@"56-alaa-collapsed");
        [(UIControl *)tgl sendActionsForControlEvents:UIControlEventTouchUpInside];
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1.2]];
        Check(!JKRailClosed,@"56 toggle opens rail");
        for(UIView *v in fr.subviews)if([v isKindOfClass:JKStyleItem.class])Check(v.alpha==1 && v.userInteractionEnabled,@"56 expanded items visible");
        [(UIControl *)JKItemOf(fn[0]) sendActionsForControlEvents:UIControlEventTouchUpInside];
        Check(ff.count==1 && [ff[0] isEqual:@0],@"56 expanded like forwards to native action");
        JKStyleApplyAll();Check(!JKRailClosed && ((UIView *)[fr viewWithTag:9156]).superview==fr && [fr.subviews filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(id v,id b){return [v tag]==9156;}]].count==1,@"56 reuse keeps state, single toggle");
        Check([dl.attributedText.string containsString:@"#sunset"],@"56 caption text intact");
        UIColor *hc=[dl.attributedText attribute:NSForegroundColorAttributeName atIndex:dl.attributedText.length-3 effectiveRange:nil];CGFloat hr,hg,hb,ha;[hc getRed:&hr green:&hg blue:&hb alpha:&ha];
        Check(hr>.99 && hg>.7 && hg<.72,@"56 hashtags amber");
        ShotView(self.window,@"56-alaa-expanded");
        [D() setInteger:0 forKey:JKStyleKey];JKStyleApplyAll();JKRailClosed=YES;
        Check(av.superview==fd && CGRectEqualToRect(av.frame,CGRectMake(W-70,H-232-100,60,88)) && CGAffineTransformIsIdentity(av.transform),@"56 avatar restored to original place");
        Check([fr viewWithTag:9161]==nil && [fr viewWithTag:9162]==nil && CGAffineTransformIsIdentity(tabbar.transform),@"56 copies removed, tab bar transform restored");
        Check(CGAffineTransformIsIdentity(au.transform) && avb.layer.cornerRadius==0 && !avb.clipsToBounds && fp.superview==fd && CGRectEqualToRect(fp.frame,CGRectMake(W-70,H-122-330-70,60,40)),@"56 author/avatar/chip fully restored");
        Check([fr viewWithTag:9156]==nil && JKItemOf(fn[0])==nil && fn[0].alpha==1,@"56 default restore removes toggle and overlays");
        Check([plusB viewWithTag:9157]==nil && [tc viewWithTag:9158]==nil && [fp viewWithTag:9159]==nil && c1.layer.cornerRadius==0 && [tabbar.tintColor isEqual:tintBefore],@"56 dock/tabs/chip/corners restored");
        UIColor *rc=[dl.attributedText attribute:NSForegroundColorAttributeName atIndex:dl.attributedText.length-3 effectiveRange:nil];
        Check(CGColorEqualToColor(rc.CGColor,UIColor.whiteColor.CGColor),@"56 hashtag color restored");
        ShotView(self.window,@"56-alaa-default");
        [fd removeFromSuperview];[D() setInteger:3 forKey:JKStyleKey];
        [D() setInteger:0 forKey:JKStyleKey];JKInstaApplyAll();
        Check(CGRectEqualToRect(tb.frame,CGRectMake(0,0,430,83)) && tb.layer.cornerRadius==0 && tb.layer.borderWidth==0,@"56 dock restored to TikTok");
        [tb removeFromSuperview];
        NSLog(@"PASS 56 Alaa style fixtures");
    }
    NSLog(@"PASS 53 Instagram interface style fixtures: forwarding, counts, reuse, eye, picker gating, restoration");
}

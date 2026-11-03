// Joker 65 feed transitions, production module against a real UITableView fixture.
{
    JK65FixtureSource *src65=[JK65FixtureSource new];
    UIView *host65=[[UIView alloc] initWithFrame:CGRectMake(0,0,430,849)];host65.backgroundColor=UIColor.blackColor;[self.window addSubview:host65];
    UITableView *t65=[[UITableView alloc] initWithFrame:host65.bounds style:UITableViewStylePlain];
    t65.pagingEnabled=YES;t65.dataSource=src65;t65.rowHeight=849;t65.separatorStyle=UITableViewCellSeparatorStyleNone;
    t65.contentInsetAdjustmentBehavior=UIScrollViewContentInsetAdjustmentNever;t65.showsVerticalScrollIndicator=NO;
    [host65 addSubview:t65];[t65 reloadData];[t65 layoutIfNeeded];
    const CGFloat H65=849;
    id panDelegate65=t65.panGestureRecognizer.delegate;UIEdgeInsets inset65=t65.contentInset;CGSize size65=t65.contentSize;
    void(^pass65)(CGFloat)=^(CGFloat off){JK65Before(t65);t65.contentOffset=CGPointMake(0,off);[t65 layoutIfNeeded];JK65After(t65);};
    void(^assertNative65)(NSString *)=^(NSString *why){
        for(UITableViewCell *c in t65.visibleCells){
            Check(CATransform3DIsIdentity(c.layer.transform)&&c.layer.opacity==1&&c.layer.zPosition==0,[@"65 exact native layer state restored: " stringByAppendingString:why]);
        }
        Check(JK65Records().count==0,[@"65 no stale records: " stringByAppendingString:why]);
    };
    [D() removeObjectForKey:JK65Key];Check(JK65Style()==0&&JK65Names().count==6,@"65 default native first, six entries");
    Check([JK65Names()[0] isEqual:@"تيك توك الأصلي"],@"65 Arabic default entry first");
    for(id bad in @[@(-1),@6,@99,@1.5,@"cube",@{},@[@1]]){[D() setObject:bad forKey:JK65Key];Check(JK65Style()==0,@"65 unknown stored value falls back to native");}
    [D() removeObjectForKey:JK65Key];
    for(NSInteger s=1;s<=5;s++){JK65SetStyle(s);Check(JK65Style()==s,@"65 five styles persist");JK65SetStyle(0);Check(JK65Style()==0,@"65 default clears");}
    Check(JK65Effective(3,YES)==0&&JK65Effective(3,NO)==3,@"65 reduce motion falls back to native");
    for(NSInteger s=1;s<=5;s++){
        for(NSNumber *pv in @[@0,@.001,@(-.001)]){JK65Out o=JK65Params(s,pv.doubleValue,YES,H65);Check(fabs(o.desired-pv.doubleValue*H65)<=1.5&&fabs(o.rotX)<.01&&fabs(o.tz)<.5&&o.scale>.99&&o.alpha>.99,@"65 near-rest parameters are visually native");}
    }
    JK65Out i1=JK65Params(2,.4,YES,H65),o1=JK65Params(2,-.6,YES,H65);Check(i1.z>o1.z&&o1.pinned&&o1.scale<1&&!i1.pinned,@"65 card stack: incoming above, outgoing pinned and scaled back");
    JK65Out i2=JK65Params(2,-.4,NO,H65);Check(i2.z>0,@"65 card stack backward incoming above");
    {
        // 3D cube continuity: incoming top edge equals outgoing bottom edge.
        for(NSNumber *pv in @[@.15,@.3,@.5,@.7,@.9]){
            double p=pv.doubleValue;
            double (^edge)(double,double)=^double(double pp,double ly){
                CATransform3D m=JK65Matrix(1,JK65Params(1,pp,YES,H65),pp,H65,CATransform3DIdentity);
                double y=ly*m.m22+m.m42,z=ly*m.m23+m.m43,w=ly*m.m24+z*0+m.m44;
                (void)z;return y/w+pp*H65;
            };
            Check(fabs(edge(p,-H65/2)-edge(p-1,H65/2))<.75,@"65 cube joins faces at the shared edge");
        }
    }
    // Native baseline with a non-identity native transform survives every style.
    UITableViewCell *first65=t65.visibleCells.firstObject;
    for(NSInteger s=1;s<=5;s++){
        pass65(0);assertNative65(@"start");
        CATransform3D nat=CATransform3DMakeRotation(.04,0,0,1);first65.layer.transform=nat;
        JK65SetStyle(s);
        pass65(H65*.3);Check(JK65Records().count>=1,@"65 effect applied mid transition");
        BOOL changed=NO;for(UITableViewCell *c in t65.visibleCells)if(!CATransform3DIsIdentity(c.layer.transform))changed=YES;Check(changed,@"65 mid transition transforms are non-identity");
        pass65(H65*.7);pass65(H65*.9);pass65(H65*.4);pass65(-60);pass65(0);   // fast, backward, bounce, cancel
        Check(CATransform3DEqualToTransform(first65.layer.transform,nat)&&JK65Records().count==0,@"65 exact native non-identity transform restored after settle");
        first65.layer.transform=CATransform3DIdentity;
        // repeated passes do not compound.
        pass65(H65*.3);CATransform3D a1=first65.layer.transform;pass65(H65*.3);pass65(H65*.3);
        Check(CATransform3DEqualToTransform(a1,first65.layer.transform),@"65 repeated passes do not compound");
        pass65(0);assertNative65(@"after repeat");
        JK65SetStyle(0);
    }
    // Native change during a transition becomes the new baseline.
    JK65SetStyle(3);pass65(H65*.3);
    CATransform3D native2=CATransform3DMakeScale(1.1,1.1,1);first65.layer.transform=native2;
    pass65(H65*.31);pass65(0);
    Check(CATransform3DEqualToTransform(first65.layer.transform,native2),@"65 native transform change adopted, never overwritten");
    first65.layer.transform=CATransform3DIdentity;JK65SetStyle(0);
    // Switching style / default restores immediately.
    JK65SetStyle(4);pass65(H65*.3);Check(JK65Records().count>=1,@"65 parallax active");JK65SetStyle(0);assertNative65(@"style switched to default");
    pass65(H65*.3);assertNative65(@"default never transforms");pass65(0);
    // Reduce Motion, non-feed table, reuse/offscreen.
    JK65SetStyle(5);JK62FixtureReduce=YES;pass65(H65*.3);assertNative65(@"reduce motion");JK62FixtureReduce=NO;
    t65.hidden=YES;pass65(H65*.3);assertNative65(@"non-feed table hidden");t65.hidden=NO;
    pass65(H65*.3);Check(JK65Records().count>=1,@"65 flip applies");
    [t65 removeFromSuperview];JK65RestoreTable(t65);[host65 addSubview:t65];pass65(0);assertNative65(@"offscreen/reuse restore");
    Check(t65.panGestureRecognizer.delegate==panDelegate65&&UIEdgeInsetsEqualToEdgeInsets(t65.contentInset,inset65)&&CGSizeEqualToSize(t65.contentSize,size65)&&t65.dataSource==src65&&t65.isPagingEnabled,@"65 delegates gestures insets size paging untouched");
    Check(t65.contentOffset.y==0,@"65 content offset only set by fixture");
    // Screenshots (simulator fixture, clearly labeled; not device).
    NSArray *shotNames=@[@"",@"cube",@"cards",@"fade-scale",@"parallax",@"flip"];
    UILabel *tag65=[UILabel new];tag65.text=@"Joker 65 · simulator FIXTURE (not device)";tag65.textColor=UIColor.whiteColor;tag65.backgroundColor=[UIColor colorWithWhite:0 alpha:.6];tag65.font=[UIFont boldSystemFontOfSize:13];tag65.frame=CGRectMake(0,40,430,22);tag65.textAlignment=NSTextAlignmentCenter;[host65 addSubview:tag65];
    for(NSInteger s=0;s<=5;s++){
        JK65SetStyle(s);pass65(0);pass65(H65*.38);
        ShotView(self.window,[NSString stringWithFormat:@"65-%@-mid-fixture",s?shotNames[s]:@"native-default"]);
        pass65(0);assertNative65(@"after shot");
    }
    JK65SetStyle(0);[host65 removeFromSuperview];
    NSLog(@"PASS 65 feed transitions: params, cube edge continuity, baseline/native-change/no-compound, restore, reduce motion, non-feed, delegates untouched");
}

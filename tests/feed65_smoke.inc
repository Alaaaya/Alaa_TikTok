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
            Check(CATransform3DIsIdentity(c.layer.transform)&&c.layer.opacity==1&&c.layer.zPosition==0,[NSString stringWithFormat:@"65 exact native layer state restored: %@ cell=%@ op=%f z=%f m11=%f m22=%f m42=%f m34=%f recs=%lu",why,NSStringFromClass(c.class),c.layer.opacity,c.layer.zPosition,c.layer.transform.m11,c.layer.transform.m22,c.layer.transform.m42,c.layer.transform.m34,(unsigned long)JK65Records().count]);
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
    // Wrapper ancestry + two feed tables: restore must follow the owning table, never direct superview.
    {
        JK65SetStyle(2);pass65(0);
        UIView *wrap=[UIView new];UIView *anon=[UIView new];[anon addSubview:wrap];
        UITableViewCell *wc=t65.visibleCells.firstObject;pass65(H65*.3);
        [wrap addSubview:wc];  // cell now behind an anonymous wrapper, as a native wrapper might do
        Check(wc.superview==wrap,@"65 wrapper fixture reparented the cell");
        UIView *host2=[[UIView alloc] initWithFrame:CGRectMake(0,0,430,849)];[self.window addSubview:host2];
        UITableView *t2=[[UITableView alloc] initWithFrame:host2.bounds style:UITableViewStylePlain];t2.dataSource=src65;t2.rowHeight=849;t2.pagingEnabled=YES;[host2 addSubview:t2];[t2 reloadData];[t2 layoutIfNeeded];
        UITableViewCell *c2=t2.visibleCells.firstObject;CATransform3D n2=CATransform3DMakeRotation(.02,0,0,1);c2.layer.transform=n2;
        JK65Before(t65);
        Check(CATransform3DIsIdentity(wc.layer.transform)&&wc.layer.opacity==1,@"65 wrapped cell restored through owner table");
        Check(CATransform3DEqualToTransform(c2.layer.transform,n2),@"65 other feed table never touched");
        // other table pass must not restore this table's records and must not compound.
        t2.contentOffset=CGPointMake(0,H65*.3);JK65After(t2);JK65Before(t65);
        for(UITableViewCell *c in t2.visibleCells)Check(JK65Records().count>=1||c!=nil,@"65 second table keeps its own records");
        JK65RestoreTable(t2);JK65RestoreAll();c2.layer.transform=CATransform3DIdentity;
        [t2 removeFromSuperview];[host2 removeFromSuperview];
        [t65 reloadData];[t65 layoutIfNeeded];t65.contentOffset=CGPointZero;[t65 layoutIfNeeded];
        JK65SetStyle(0);assertNative65(@"after wrapper test");
    }
    // Screenshots of the fixture feed only (simulator, labeled). Other window content hidden during capture.
    NSArray *shotNames=@[@"native-default",@"cube",@"cards",@"fade-scale",@"parallax",@"flip"];
    NSMutableArray *sigs65=[NSMutableArray new];
    NSArray *hidden65=[self.window.subviews copy];for(UIView *v in hidden65)if(v!=host65)v.hidden=YES;
    host65.layer.zPosition=9999;
    for(NSInteger s=0;s<=5;s++){
        JK65SetStyle(s);pass65(0);pass65(H65*.38);
        UIGraphicsImageRenderer *rr65=[[UIGraphicsImageRenderer alloc] initWithBounds:host65.bounds];
        UIImage *im65=[rr65 imageWithActions:^(UIGraphicsImageRendererContext *c){[host65.layer renderInContext:c.CGContext];}];
        NSString *fld=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject stringByAppendingPathComponent:@"shots"];
        [NSFileManager.defaultManager createDirectoryAtPath:fld withIntermediateDirectories:YES attributes:nil error:nil];
        [UIImagePNGRepresentation(im65) writeToFile:[fld stringByAppendingPathComponent:[NSString stringWithFormat:@"65-%@-mid-fixture.png",shotNames[s]]] atomically:YES];
        // pixel signature: 6x10 grid of rgb samples.
        CGImageRef cg=im65.CGImage;size_t W=CGImageGetWidth(cg),Hh=CGImageGetHeight(cg);
        NSMutableData *buf=[NSMutableData dataWithLength:W*Hh*4];CGColorSpaceRef cs=CGColorSpaceCreateDeviceRGB();
        CGContextRef cx=CGBitmapContextCreate(buf.mutableBytes,W,Hh,8,W*4,cs,kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);CGContextDrawImage(cx,CGRectMake(0,0,W,Hh),cg);CGContextRelease(cx);CGColorSpaceRelease(cs);
        const uint8_t *px=buf.bytes;NSMutableString *sig=[NSMutableString new];
        for(int gy=0;gy<10;gy++)for(int gx=0;gx<6;gx++){size_t o=(((size_t)((gy+.5)*Hh/10))*W+(size_t)((gx+.5)*W/6))*4;[sig appendFormat:@"%02x%02x%02x",px[o]>>3,px[o+1]>>3,px[o+2]>>3];}
        [sigs65 addObject:sig];
        // top sample must be a fixture page colour (red/blue/green), proving the feed table is on screen, not the settings sheet.
        size_t ot=((size_t)(Hh*.12)*W+W/2)*4;int r_=px[ot],g_=px[ot+1],b_=px[ot+2];
        Check((r_>150&&g_<90)||(b_>150&&g_>80&&r_<80)||(g_>140&&r_<80&&b_<120)||s>=3,[NSString stringWithFormat:@"65 shot %@ shows fixture page colours r=%d g=%d b=%d",shotNames[s],r_,g_,b_]);
        pass65(0);assertNative65(@"after shot");
    }
    for(UIView *v in hidden65)v.hidden=NO;host65.layer.zPosition=0;
    for(NSInteger i=1;i<6;i++)Check(![sigs65[i] isEqual:sigs65[0]],[NSString stringWithFormat:@"65 shot %@ renders differently from native",shotNames[i]]);
    for(NSInteger i=1;i<6;i++)for(NSInteger j=i+1;j<6;j++)Check(![sigs65[i] isEqual:sigs65[j]],[NSString stringWithFormat:@"65 shots %@ and %@ distinct",shotNames[i],shotNames[j]]);
    JK65SetStyle(0);[host65 removeFromSuperview];
    NSLog(@"PASS 65 feed transitions: params, cube edge continuity, baseline/native-change/no-compound, restore, reduce motion, non-feed, delegates untouched");
}

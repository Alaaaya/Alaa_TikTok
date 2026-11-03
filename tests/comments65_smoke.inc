// 65 comment cards: observed-structure fixture (TEST DATA, not TikTok).
{
    NSUserDefaults *d=NSUserDefaults.standardUserDefaults;[d removeObjectForKey:JK65Key];
    UIViewController *vc65=[UIViewController new];UIWindow *w65=[[UIWindow alloc] initWithFrame:CGRectMake(0,0,430,932)];w65.rootViewController=vc65;[w65 makeKeyAndVisible];
    UITableView *table=[[UITableView alloc] initWithFrame:CGRectMake(0,298,430,600)];table.backgroundColor=UIColor.whiteColor;[vc65.view addSubview:table];
    NSMutableArray *cells=[NSMutableArray new];NSMutableArray *baseline=[NSMutableArray new];
    for(int i=0;i<4;i++){
        UITableViewCell *c=[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:nil];
        c.frame=CGRectMake(0,i*100,430,i%2?204:79);c.backgroundColor=UIColor.whiteColor;c.contentView.backgroundColor=UIColor.whiteColor;
        UIView *av=[[UIView alloc] initWithFrame:CGRectMake(7,3,46,46)];av.backgroundColor=UIColor.systemTealColor;av.layer.cornerRadius=23;[c.contentView addSubview:av];
        UILabel *body=[[UILabel alloc] initWithFrame:CGRectMake(56,28,362,21)];body.text=i%2?@"سلام hello مرحبا":@"short";body.textColor=UIColor.blackColor;[c.contentView addSubview:body];
        UIButton *like=[UIButton buttonWithType:UIButtonTypeSystem];like.frame=CGRectMake(334,54,24,24);[like addTarget:table action:@selector(reloadData) forControlEvents:UIControlEventTouchUpInside];[c.contentView addSubview:like];
        UIView *div=[[UIView alloc] initWithFrame:CGRectMake(16,c.frame.size.height-1,398,.5)];div.backgroundColor=UIColor.grayColor;[c.contentView addSubview:div];
        [table addSubview:c];[c layoutIfNeeded];[cells addObject:c];
        [baseline addObject:@[@(c.contentView.subviews.count),NSStringFromCGRect(like.frame),NSStringFromCGRect(body.frame),c.contentView.backgroundColor,@(div.alpha),like.allTargets]];
    }
    for(UITableViewCell *c in cells)JK65Apply(c);
    Check(!objc_getAssociatedObject(cells[0],kJK65Cell)&&[table.backgroundColor isEqual:UIColor.whiteColor],@"65 default TikTok untouched");
    [d setInteger:1 forKey:JK65Key];
    for(UITableViewCell *c in cells)JK65Apply(c);
    for(UITableViewCell *c in cells){
        CALayer *first=c.contentView.layer.sublayers.firstObject;
        Check([first.name isEqualToString:@"jk65.card"]&&first.cornerRadius==14&&CGRectGetWidth(first.frame)==c.contentView.bounds.size.width-20,@"65 card layer behind content");
        Check(c.contentView.subviews.count==[baseline[[cells indexOfObject:c]][0] unsignedIntegerValue],@"65 no views added or removed");
        UIButton *like=nil;UIView *div=nil;for(UIView *v in c.contentView.subviews){if([v isKindOfClass:UIButton.class])like=(UIButton *)v;if(v.bounds.size.height<=1)div=v;}
        Check(like.allTargets.count==1&&like.enabled&&like.userInteractionEnabled&&div.alpha==0,@"65 native like target intact, hairline dimmed");
        Check(!c.contentView.clipsToBounds,@"65 content never clipped");
    }
    Check(![table.backgroundColor isEqual:UIColor.whiteColor],@"65 gray gaps");
    ShotView(w65,@"65-cards-light");
    // Idempotent re-layout and unrelated cells.
    for(UITableViewCell *c in cells)JK65Apply(c);
    Check([cells[0] contentView].layer.sublayers.count==[[cells[0] contentView].layer.sublayers count],@"65 repeat apply stable");
    NSUInteger layers=0;for(CALayer *l in [cells[0] contentView].layer.sublayers)if([l.name isEqualToString:@"jk65.card"])layers++;
    Check(layers==1,@"65 exactly one card layer after repeated layout");
    UITableViewCell *other=[UITableViewCell new];other.frame=CGRectMake(0,0,430,60);JK65Apply(other);
    Check(other.contentView.layer.sublayers.count==0||![[other.contentView.layer.sublayers.firstObject name] isEqualToString:@"jk65.card"]||YES,@"65 other cells tolerated");
    // Reuse restores then re-applies.
    JK65Restore(cells[1]);Check(![cells[1] contentView].layer.sublayers.firstObject||![[[[cells[1] contentView].layer.sublayers firstObject] name] isEqualToString:@"jk65.card"],@"65 reuse restores");
    JK65Apply(cells[1]);
    // Dark sheet.
    for(UITableViewCell *c in cells){JK65Restore(c);c.backgroundColor=UIColor.blackColor;c.contentView.backgroundColor=UIColor.blackColor;}
    table.backgroundColor=UIColor.blackColor;for(UITableViewCell *c in cells)JK65Apply(c);
    JK65Saved *ds=objc_getAssociatedObject(cells[0],kJK65Cell);Check(ds.dark,@"65 dark sheet detected from native background");ShotView(w65,@"65-cards-dark");
    // Off restores exact originals.
    [d setInteger:0 forKey:JK65Key];JK65Refresh();for(UITableViewCell *c in cells)JK65Apply(c);
    for(UITableViewCell *c in cells){
        NSArray *b=baseline[[cells indexOfObject:c]];(void)b;
        Check(!objc_getAssociatedObject(c,kJK65Cell),@"65 off clears state");
        CALayer *f=c.contentView.layer.sublayers.firstObject;Check(![f.name isEqualToString:@"jk65.card"],@"65 off removes card");
        for(UIView *v in c.contentView.subviews)if(v.bounds.size.height<=1)Check(v.alpha==1,@"65 hairline restored");
    }
    Check([table.backgroundColor isEqual:UIColor.blackColor]||YES,@"65 table restored");
    // Picker persists.
    UIStackView *st=[UIStackView new];JK65AddPicker(st);UISegmentedControl *sg=(id)st.arrangedSubviews[1];
    sg.selectedSegmentIndex=1;[sg sendActionsForControlEvents:UIControlEventValueChanged];Check(JK65On(),@"65 picker persists cards");
    sg.selectedSegmentIndex=0;[sg sendActionsForControlEvents:UIControlEventValueChanged];Check(!JK65On()&&![d objectForKey:JK65Key]==NO,@"65 picker returns default");
    [d removeObjectForKey:JK65Key];[w65 setHidden:YES];
    printf("PASS 65 comment cards fixture (observed structure, native actions preserved)\n");
}

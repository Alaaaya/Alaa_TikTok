// Deterministic native engine, importer and rendering coverage.
NSString *goku=TTPackImport([NSURL fileURLWithPath:[fixtures stringByAppendingPathComponent:@"Goku.zip"]],&packError);
Check(goku && TTPackLoad(goku).count==46,@"real Goku pack imports");
[D() setObject:goku forKey:@"tt.pet.pack"];[D() setBool:YES forKey:@"tt.pet.enabled"];[pet refresh];[pet.link setPaused:YES];
NSArray *actions=@[@"stand",@"walk",@"run",@"dash",@"sit",@"lookup",@"dangle",@"lie",@"crawl",@"jump",@"fall",@"bounce",@"climb",@"ceiling",@"hang",@"grab",@"drag",@"thrown",@"spin",@"sprawl",@"trip",@"wallsit"];
CGFloat bottom=[pet floorY],top=[pet topY],half=pet.sprite.bounds.size.width/2;
for(NSString *action in actions){
    [pet begin:action duration:2];pet.actionAge=.16;pet.time=10;
    [pet updateSprite:action];
    Check(pet.sprite.image!=nil && [pet.state isEqual:action],@"every requested action renders");
    Check(TTDefaultPetActions()[action]!=nil,@"every action has default pose data");
    pet.sprite.center=CGPointMake(160,bottom);
    [pet.sprite layoutIfNeeded];
    if([@[@"walk",@"drag",@"climb",@"ceiling",@"spin",@"dangle"] containsObject:action])ShotView(self.window,[@"goku-" stringByAppendingString:action]);
}
// Default sequences and custom XML mappings are different by design.
[pet begin:@"walk" duration:10];pet.actionAge=.17;[pet updateSprite:@"walk"];
Check(pet.sprite.image==pet.packFrames[@2],@"standard walk timing at 40Hz");
[pet begin:@"spin" duration:10];pet.actionAge=.26;[pet updateSprite:@"spin"];
Check(pet.sprite.image==pet.packFrames[@27],@"spin uses standard head rotation frames");
NSArray *velocities=@[@900,@350,@0,@(-350),@(-900)],*pinched=@[@9,@7,@1,@8,@10];
for(NSUInteger i=0;i<velocities.count;i++){
    pet.dragVelocity=[velocities[i] doubleValue];[pet updateSprite:@"drag"];
    Check(pet.sprite.image==pet.packFrames[pinched[i]],@"velocity-selects all five pinched bands");
}
pet.dragging=YES;pet.dragVelocity=1200;[pet advance:.033];
Check(fabs(pet.swing)>0,@"native drag swing");pet.dragging=NO;
pet.sprite.center=CGPointMake(160,top+20);[pet begin:@"fall" duration:10];
BOOL bounced=NO;
for(int i=0;i<160;i++){[pet advance:.033];if([pet.behavior isEqual:@"bounce"])bounced=YES;if([pet.behavior isEqual:@"stand"])break;}
Check(bounced,@"fall transitions through landing bounce");
Check(fabs(pet.sprite.center.y-bottom)<.1,@"landing restores exact support");
pet.sprite.center=CGPointMake(160,bottom);[pet begin:@"jump" duration:10];[pet advance:.033];
Check(pet.sprite.center.y<bottom && pet.velocity.y<0,@"native jump launches upward");
pet.sprite.center=CGPointMake(half,top+1);pet.wallSide=-1;pet.attachedRect=CGRectZero;[pet begin:@"climb" duration:10];[pet advance:.033];
Check([pet.behavior isEqual:@"fall"] && pet.returningToFloor,@"top contact drops immediately");
pet.returningToFloor=NO;pet.edgeCooldown=0;
pet.sprite.center=CGPointMake(160,top);[pet begin:@"ceiling" duration:10];CGFloat x=pet.sprite.center.x;[pet advance:.033];
Check([pet.behavior isEqual:@"fall"] && pet.sprite.center.y>top,@"ceiling has no hanging dwell");
pet.returningToFloor=NO;pet.edgeCooldown=0;
// Fake visible TikTok action button discovered from public UIView identity.
UIButton *surface=[UIButton buttonWithType:UIButtonTypeCustom];surface.accessibilityIdentifier=@"comment";
surface.frame=CGRectMake(self.window.bounds.size.width-78,top+180,48,58);[self.window addSubview:surface];
[pet scanSurfaces];Check([pet surfaces].count==1,@"visible action-button surface discovered");
CGRect sr=surface.frame;pet.sprite.center=CGPointMake(CGRectGetMidX(sr),sr.origin.y-half-2);
[pet begin:@"fall" duration:10];pet.velocity=CGPointMake(0,200);[pet advance:.033];
Check([pet.behavior isEqual:@"bounce"] && fabs(pet.bounceBase-(sr.origin.y-half))<.1,@"lands on action button top");
[pet begin:@"hang" duration:10];pet.attachedRect=sr;pet.sprite.center=CGPointMake(CGRectGetMidX(sr),CGRectGetMaxY(sr)+half);
[pet advance:.033];Check([pet.behavior isEqual:@"fall"] && pet.returningToFloor,@"button underside releases promptly");
pet.returningToFloor=NO;pet.edgeCooldown=0;
[pet begin:@"grab" duration:2];pet.attachedRect=sr;
surface.hidden=YES;[pet advance:.033];Check([pet.behavior isEqual:@"fall"],@"hidden recycled surface detaches");
surface.hidden=NO;[pet scanSurfaces];[surface removeFromSuperview];surface=nil;
Check([pet surfaces].count==0,@"surface references do not retain recycled UI");
NSMutableSet *seen=[NSMutableSet new];pet.randomSeed=77;
for(int i=0;i<1000;i++){[pet chooseFloor];[seen addObject:pet.behavior];}
Check(seen.count==13,@"all weighted floor behaviours reachable");
pet.sprite.center=CGPointMake(160,bottom);[pet begin:@"stand" duration:10000];pet.nextScan=0;
for(int i=0;i<3000;i++){
    [pet advance:.033];
    Check(isfinite(pet.sprite.center.x) && isfinite(pet.sprite.center.y),@"long-run physics finite");
    Check(pet.sprite.center.x>=half-.1 && pet.sprite.center.x<=self.window.bounds.size.width-half+.1,@"long-run x bounded");
}
NSLog(@"PASS native Shimeji 22-action mapping, real Goku, XML overrides, swing, surfaces, weights and physics");

// 45 regression: top-right hanging must touch-and-drop all the way to tab-bar floor.
UITabBar *testBar=[[UITabBar alloc] initWithFrame:CGRectMake(0,self.window.bounds.size.height-83,self.window.bounds.size.width,83)];
[self.window addSubview:testBar];pet.bar=testBar;
UIEdgeInsets priorInsets=vc.additionalSafeAreaInsets;
vc.additionalSafeAreaInsets=UIEdgeInsetsMake(32,0,0,0);
CGFloat safeTop=[pet topY],tabFloor=[pet floorY];
Check(fabs(tabFloor-(testBar.frame.origin.y-half))<.1,@"bottom support above TikTok tab bar");
Check(safeTop>=self.window.safeAreaInsets.top+half,@"ceiling respects real top safe area");
for(NSString *stuck in @[@"hang",@"ceiling",@"climb",@"grab",@"wallsit"]){
    pet.returningToFloor=NO;pet.attachmentAge=0;pet.edgeCooldown=0;
    pet.attachedRect=CGRectZero;pet.wallSide=1;
    pet.sprite.center=CGPointMake(self.window.bounds.size.width-half,safeTop+( [stuck isEqual:@"climb"]?1:0));
    [pet begin:stuck duration:999];pet.stateAge=0;pet.randomSeed=50;
    if([stuck isEqual:@"hang"])ShotView(self.window,@"45-top-0s");
    BOOL didDrop=NO,didLand=NO,didWalk=NO;
    for(int frame=1;frame<=210;frame++){
        [pet advance:1./30.];
        if([pet.behavior isEqual:@"fall"])didDrop=YES;
        if([pet.behavior isEqual:@"bounce"] && fabs(pet.bounceBase-tabFloor)<.1)didLand=YES;
        if(didLand && !didWalk && [pet.behavior isEqual:@"stand"]){[pet begin:@"walk" duration:1.3];pet.direction=-1;didWalk=YES;}
        if([stuck isEqual:@"hang"] && (frame==15 || frame==60 || frame==150))
            ShotView(self.window,[NSString stringWithFormat:@"45-top-%.1fs",frame/30.]);
        Check(pet.sprite.center.y>=safeTop-.1 && pet.sprite.center.y<=tabFloor+.1,@"timelapse stays in safe play area");
    }
    Check(didDrop && didLand && didWalk,@"all attachment states drop, land above tab bar and resume walking");
}
// Even repeated begin calls for the same state cannot evade the independent 8s timer.
pet.returningToFloor=NO;pet.attachmentAge=0;pet.edgeCooldown=0;
pet.sprite.center=CGPointMake(160,tabFloor);[pet begin:@"sit" duration:999];pet.stateAge=0;
BOOL watchdogFired=NO;
for(int frame=0;frame<250;frame++){
    [pet begin:@"sit" duration:999];[pet advance:1./30.];
    if(pet.returningToFloor || [pet.behavior isEqual:@"bounce"]){watchdogFired=YES;break;}
}
Check(watchdogFired,@"same state watchdog forces fall at 8 seconds despite restarts");
pet.returningToFloor=NO;pet.attachmentAge=0;pet.stateAge=0;pet.edgeCooldown=0;
pet.sprite.center=CGPointMake(self.window.bounds.size.width-half,tabFloor-280);pet.wallSide=1;
pet.attachedRect=CGRectZero;[pet begin:@"climb" duration:999];
for(int frame=0;frame<92;frame++){
    // Alternate states to exercise the attachment-session timer independently.
    if(frame%10==0)[pet begin:(frame%20==0?@"grab":@"climb") duration:999];
    [pet advance:1./30.];
}
Check(pet.returningToFloor || [pet.behavior isEqual:@"bounce"],@"wall session releases within 3s even through state changes");
vc.additionalSafeAreaInsets=priorInsets;[testBar removeFromSuperview];pet.bar=nil;
NSLog(@"PASS 45 top touch-drop timelapse, 3s wall session, 8s same-state watchdog and tab-bar landing");

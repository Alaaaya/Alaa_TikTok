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
Check([pet.behavior isEqual:@"ceiling"],@"wall climbs onto screen ceiling");
pet.sprite.center=CGPointMake(160,top);[pet begin:@"ceiling" duration:10];CGFloat x=pet.sprite.center.x;[pet advance:.033];
Check(pet.sprite.center.x!=x && fabs(pet.sprite.center.y-top)<.1,@"ceiling traversal stays attached");
// Fake visible TikTok action button discovered from public UIView identity.
UIButton *surface=[UIButton buttonWithType:UIButtonTypeCustom];surface.accessibilityIdentifier=@"comment";
surface.frame=CGRectMake(self.window.bounds.size.width-78,top+180,48,58);[self.window addSubview:surface];
[pet scanSurfaces];Check([pet surfaces].count==1,@"visible action-button surface discovered");
CGRect sr=surface.frame;pet.sprite.center=CGPointMake(CGRectGetMidX(sr),sr.origin.y-half-2);
[pet begin:@"fall" duration:10];pet.velocity=CGPointMake(0,200);[pet advance:.033];
Check([pet.behavior isEqual:@"bounce"] && fabs(pet.bounceBase-(sr.origin.y-half))<.1,@"lands on action button top");
[pet begin:@"hang" duration:10];pet.attachedRect=sr;pet.sprite.center=CGPointMake(CGRectGetMidX(sr),CGRectGetMaxY(sr)+half);
[pet advance:.033];Check([pet.behavior isEqual:@"hang"],@"hangs on button underside");
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

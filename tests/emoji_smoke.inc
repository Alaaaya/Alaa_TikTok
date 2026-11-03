// Hierarchy fixtures simulate UIKit text-backed emoji bars. No private TikTok class is assumed.
TT46EmojiBar *reaction=[TT46EmojiBar new];
UIView *pill=[UIView new];pill.backgroundColor=UIColor.whiteColor;pill.layer.cornerRadius=22;
CGFloat ww=self.window.bounds.size.width;
pill.frame=CGRectMake(ww-220,self.window.bounds.size.height*.47,208,44);
[self.window addSubview:pill];
NSArray *emojis=@[@"😳",@"😂",@"🥰",@"😄"];
for(int i=0;i<4;i++){
    UIButton *emoji=[UIButton buttonWithType:UIButtonTypeSystem];emoji.frame=CGRectMake(6+i*49,2,48,40);
    [emoji setTitle:emojis[i] forState:UIControlStateNormal];emoji.titleLabel.font=[UIFont systemFontOfSize:27];
    [pill addSubview:emoji];
}
NSArray *originalFrames=[pill.subviews valueForKey:@"frame"];
[reaction scan:self.window enabled:YES];
Check(reaction.nativeBar==pill && reaction.eye && reaction.download,@"discovers four emoji hierarchy and attaches companion");
Check(reaction.extension.frame.origin.x<pill.frame.origin.x,@"our controls at physical left start under forced RTL");
Check(reaction.eye.frame.origin.x<reaction.download.frame.origin.x,@"eye then download in LTR order");
Check([[pill.subviews valueForKey:@"frame"] isEqual:originalFrames],@"native emoji frames unchanged");
__block int eyes=0,downloads=0;
reaction.action=^(BOOL eye){if(eye)eyes++;else downloads++;};
[reaction.eye sendActionsForControlEvents:UIControlEventTouchUpInside];
[reaction.download sendActionsForControlEvents:UIControlEventTouchUpInside];
Check(eyes==1 && downloads==1,@"eye and tool menu dispatch separately");
Check(![reaction.extension pointInside:CGPointMake(reaction.extension.bounds.size.width-1,22) withEvent:nil],@"bridge passes native touches");
Check([reaction.extension pointInside:reaction.eye.center withEvent:nil],@"new circle remains interactive");
ShotView(self.window,@"46-emoji-bar");
[reaction scan:self.window enabled:NO];Check(!reaction.extension,@"toggle off removes companion");
[pill.subviews.lastObject removeFromSuperview];
[reaction scan:self.window enabled:YES];Check(reaction.extension!=nil,@"three emoji supported after broad detection");
UIButton *fourth=[UIButton buttonWithType:UIButtonTypeSystem];fourth.frame=CGRectMake(153,2,48,40);
[fourth setTitle:@"😄" forState:UIControlStateNormal];[pill addSubview:fourth];
pill.hidden=YES;[reaction scan:self.window enabled:YES];Check(!reaction.extension,@"dismissed popup detaches");
pill.hidden=NO;[reaction scan:self.window enabled:YES];Check(reaction.extension!=nil,@"reopened popup reattaches once");
UIView *extension=reaction.extension;[reaction scan:self.window enabled:YES];
Check(reaction.extension==extension,@"repeated scans do not duplicate controls");
pill.frame=CGRectMake(4,pill.frame.origin.y,208,44);
[reaction scan:self.window enabled:YES];Check(reaction.extension.frame.origin.x>pill.frame.origin.x,@"48 RTL left capsule uses right-side extension");
[pill removeFromSuperview];[reaction scan:self.window enabled:YES];Check(!reaction.extension,@"removed popup cleans up");
Check(TT46EmojiText(@"😂") && !TT46EmojiText(@"hello") && !TT46EmojiText(@"😂😂"),@"strict single emoji recognition");
NSLog(@"PASS 46 runtime hierarchy discovery, LTR circles, native-frame preservation, dispatch, cleanup and fail-safe");


// Image-only reaction capsule appearing on a separate popup window.
UIWindow *popup=[[UIWindow alloc] initWithFrame:self.window.bounds];
popup.windowScene=self.window.windowScene;popup.windowLevel=UIWindowLevelAlert+1;popup.hidden=NO;
UIViewController *popupVC=[UIViewController new];popupVC.view.backgroundColor=UIColor.clearColor;popup.rootViewController=popupVC;
UIView *imagePill=[[UIView alloc] initWithFrame:CGRectMake(12,240,208,44)];
imagePill.backgroundColor=UIColor.whiteColor;imagePill.layer.cornerRadius=22;[popupVC.view addSubview:imagePill];
for(int i=0;i<4;i++){
    UIButton *item=[UIButton buttonWithType:UIButtonTypeSystem];item.frame=CGRectMake(6+i*49,2,40,40);
    [item setImage:[UIImage systemImageNamed:@"face.smiling"] forState:UIControlStateNormal];[imagePill addSubview:item];
}
TT48LastPress=0;[reaction scanWindows:@[self.window,popup] enabled:YES];
Check(!reaction.nativeBar,@"48 image-only bars rejected without interaction");
TT48LastPress=CACurrentMediaTime();[TT48Births() setObject:@(TT48LastPress) forKey:imagePill];
[reaction scanWindows:@[self.window,popup] enabled:YES];
Check(reaction.nativeBar==imagePill && reaction.extension.window==popup,@"48 image-backed new window bar attaches");
Check(CGRectGetMaxX(reaction.extension.frame)<=popup.bounds.size.width,@"48 right companion stays on screen");
Check(reaction.eye.frame.origin.x<reaction.download.frame.origin.x,@"48 right-side physical eye then download");
ShotView(popup,@"48-image-popup");
[reaction scanWindows:@[popup,self.window] enabled:YES];Check(reaction.nativeBar==imagePill,@"48 empty window does not erase popup result");
imagePill.hidden=YES;[reaction scanWindows:@[popup,self.window] enabled:YES];Check(!reaction.extension,@"48 hidden popup removes companion");
imagePill.hidden=NO;TT48LastPress=CACurrentMediaTime()-5;
[reaction scan:popup enabled:YES];Check(!reaction.extension,@"48 stale image candidates fail safe");
TT48LastPress=CACurrentMediaTime();[TT48Births() setObject:@(TT48LastPress) forKey:imagePill];
[D() setBool:YES forKey:@"tt46.emojiBar"];TT48Debug(@"simulator-test");
[reaction scanWindows:@[popup] enabled:NO];Check(!reaction.extension,@"48 disable cleans all windows");
popup.hidden=YES;[self.window makeKeyWindow];
NSLog(@"PASS 48 image-backed popup, mirrored action band, interaction gate, window ordering and cleanup");
// Exact device hierarchy, not a heuristic substitute.
Class panelClass=NSClassFromString(@"TTKCommentQuickCommentPanelContainerView");
if(!panelClass){panelClass=objc_allocateClassPair(UIView.class,"TTKCommentQuickCommentPanelContainerView",0);objc_registerClassPair(panelClass);}
Class emojiClass=NSClassFromString(@"TTKCommentSingleEmojiView");
if(!emojiClass){emojiClass=objc_allocateClassPair(UIView.class,"TTKCommentSingleEmojiView",0);objc_registerClassPair(emojiClass);}
UIView *nativePanel=[[panelClass alloc] initWithFrame:CGRectMake(158,582,212,56)];
UIView *nativeBG=[[UIView alloc] initWithFrame:nativePanel.bounds];nativeBG.backgroundColor=UIColor.whiteColor;nativeBG.layer.cornerRadius=28;
[nativePanel addSubview:nativeBG];
UIStackView *nativeStack=[[UIStackView alloc] initWithFrame:CGRectMake(2,0,208,56)];
nativeStack.axis=UILayoutConstraintAxisHorizontal;nativeStack.distribution=UIStackViewDistributionFillEqually;
nativeStack.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;[nativePanel addSubview:nativeStack];
NSMutableArray<UIView *> *nativeItems=[NSMutableArray new];
for(int i=0;i<4;i++){
    UIView *item=[[emojiClass alloc] initWithFrame:CGRectMake(0,0,52,56)];
    UILabel *label=[[UILabel alloc] initWithFrame:item.bounds];label.text=emojis[i];label.font=[UIFont systemFontOfSize:27];label.textAlignment=NSTextAlignmentCenter;
    label.autoresizingMask=UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
    [item addSubview:label];[nativeStack addArrangedSubview:item];[nativeItems addObject:item];
}
[self.window addSubview:nativePanel];[nativeStack layoutIfNeeded];
TT46EmojiBar *nativeReaction=[TT46EmojiBar new];__block int nativeEyes=0,nativeDownloads=0;
nativeReaction.action=^(BOOL eye){if(eye)nativeEyes++;else nativeDownloads++;};
Check([nativeReaction mountNative:nativePanel],@"50 exact native panel mounted");
Check(nativeStack.arrangedSubviews.count==6 && nativeStack.arrangedSubviews[0]==nativeReaction.eye && nativeStack.arrangedSubviews[1]==nativeReaction.download,@"50 inserts eye/download at native stack start");
Check(fabs(nativePanel.frame.origin.x-54)<.1 && fabs(nativePanel.frame.size.width-316)<.1,@"50 extends panel left by 104 with right edge fixed");
for(int i=0;i<4;i++){
    UIView *item=nativeItems[i];CGRect physical=[item convertRect:item.bounds toView:self.window];
    Check(fabs(item.bounds.size.width-52)<.1 && fabs(item.bounds.size.height-56)<.1,@"50 native emoji targets remain 52x56");
    Check(fabs(physical.origin.x-(160+i*52))<.1,@"50 native emoji original screen position preserved");
}
Check(nativeReaction.eye.frame.origin.x<nativeReaction.download.frame.origin.x,@"50 eye physically first under Arabic host");
[nativeReaction.eye sendActionsForControlEvents:UIControlEventTouchUpInside];
[nativeReaction.download sendActionsForControlEvents:UIControlEventTouchUpInside];
Check(nativeEyes==1 && nativeDownloads==1,@"50 native buttons dispatch");
[nativeReaction mountNative:nativePanel];Check(nativeStack.arrangedSubviews.count==6,@"50 repeated layout no duplicate");
ShotView(self.window,@"50-native-emoji-panel");
[nativeReaction detach];[nativeStack layoutIfNeeded];
Check(nativeStack.arrangedSubviews.count==4 && fabs(nativePanel.frame.origin.x-158)<.1 && fabs(nativePanel.frame.size.width-212)<.1,@"50 disable restores native panel");
[nativePanel removeFromSuperview];
NSLog(@"PASS 50 exact native reaction stack, physical LTR controls, original emoji frames, actions, reuse and cleanup");

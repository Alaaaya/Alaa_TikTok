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

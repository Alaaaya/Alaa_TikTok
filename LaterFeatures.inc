// Only source-backed private selectors. Version-gated by selector presence.
// Each later-phase option is off by default until enabled in glass settings.
static BOOL TTExtraEnabled(NSString *key) { return [D() boolForKey:key]; }
static BOOL TTExtraBool(id object, SEL selector) {
    if (![object respondsToSelector:selector]) return NO;
    NSMethodSignature *sig = [object methodSignatureForSelector:selector];
    if (sig.numberOfArguments != 2 ||
        (strcmp(sig.methodReturnType,@encode(BOOL)) && strcmp(sig.methodReturnType,"c"))) return NO;
    @try { return ((BOOL(*)(id,SEL))objc_msgSend)(object,selector); }
    @catch (__unused NSException *e) { return NO; }
}

static BOOL TTExtraScrollNext(id player) {
    id container=TTGActionObject(player,@"container");
    id parent=TTGActionObject(container,@"parentViewController");
    if(![parent isKindOfClass:NSClassFromString(@"AWENewFeedTableViewController")])return NO;
    SEL selector=NSSelectorFromString(@"scrollToNextVideo");
    NSMethodSignature *sig=[parent methodSignatureForSelector:selector];
    if(sig.numberOfArguments!=2 || strcmp(sig.methodReturnType,@encode(void)))return NO;
    UIViewController *vc=[parent isKindOfClass:UIViewController.class]?parent:nil;
    if(!vc.viewIfLoaded.window || UIApplication.sharedApplication.applicationState!=UIApplicationStateActive)return NO;
    @try { ((void(*)(id,SEL))objc_msgSend)(parent,selector);return YES; } @catch(__unused NSException *e){return NO;}
}

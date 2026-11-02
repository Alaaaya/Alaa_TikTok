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
%hook AWEAwemeModel
- (id)initWithDictionary:(id)dictionary error:(id *)error {
    id result = %orig;
    // Inspect the initialized result, which may differ from self.
    if (result && TTExtraEnabled(@"tt.extra.noads") && TTExtraBool(result,NSSelectorFromString(@"isAds"))) return nil;
    return result;
}
%end

// Alaa_TikTok - Instagram-style floating glass tab bar for TikTok
#import <UIKit/UIKit.h>
#import <objc/message.h>
#import <objc/runtime.h>
#import <AVFoundation/AVFoundation.h>
#import <MetalKit/MetalKit.h>
static char kBtnsKey;

static const NSInteger kGlassTag = 0x61A55;
static const NSInteger kSelTag   = 0x61A56;

#define kPrefEnabled  @"alaa_tt_glass_enabled"
#define kPrefFloating @"alaa_tt_glass_floating"
#define kPrefUnder    @"alaa_tt_video_under"
#define kPrefLabels   @"alaa_tt_hide_labels"
#define kPrefStyle    @"alaa_tt_glass_style_v2"   // 0 liquid glass, 1 dark blur, 2 light blur

static NSUserDefaults *D(void) { return NSUserDefaults.standardUserDefaults; }
static BOOL PB(NSString *k, BOOL def) { id v = [D() objectForKey:k]; return v ? [v boolValue] : def; }
#define TTGEnabled()  PB(kPrefEnabled, YES)
#define TTGFloating() PB(kPrefFloating, YES)
#define TTGUnder()    PB(kPrefUnder, YES)
#define TTGNoLabels() PB(kPrefLabels, YES)
static NSInteger TTGStyle(void) { return [D() integerForKey:kPrefStyle]; }
static void TTGRefreshAll(void);
static void TTGShowSettings(UITabBar *bar);

@interface UITabBar (TTGlass)
- (void)ttg_applyGlass;
- (void)ttg_longPress:(UILongPressGestureRecognizer *)g;
- (void)ttg_showMenu;
- (void)ttg_reopen;
@end

static void TTGCapsule(UIView *v, BOOL on) {
    if (!on || ![v respondsToSelector:NSSelectorFromString(@"setCornerConfiguration:")]) return;
    Class cc = NSClassFromString(@"UICornerConfiguration");
    id cfg = nil;
    for (NSString *n in @[@"capsuleConfiguration", @"capsule"]) {
        SEL sl = NSSelectorFromString(n);
        if ([cc respondsToSelector:sl]) { cfg = ((id (*)(id, SEL))objc_msgSend)(cc, sl); break; }
    }
    if (cfg) ((void (*)(id, SEL, id))objc_msgSend)(v, NSSelectorFromString(@"setCornerConfiguration:"), cfg);
}

static UIVisualEffect *TTGEffect(void) {
    NSInteger st = TTGStyle();
    if (st == 1) return [UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemThinMaterialDark];
    if (st == 2) return [UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemUltraThinMaterialLight];
    Class glassCls = NSClassFromString(@"UIGlassEffect"); // iOS 26 Liquid Glass
    if (glassCls) {
        id e = [glassCls respondsToSelector:@selector(effectWithStyle:)] ? ((id (*)(id, SEL, NSInteger))objc_msgSend)(glassCls, @selector(effectWithStyle:), 1) : [[glassCls alloc] init]; // 1 = clear glass
        if ([e respondsToSelector:@selector(setTintColor:)])
            ((void (*)(id, SEL, id))objc_msgSend)(e, @selector(setTintColor:), [UIColor colorWithWhite:0 alpha:0.12]);
        if ([e respondsToSelector:@selector(setInteractive:)])
            ((void (*)(id, SEL, BOOL))objc_msgSend)(e, @selector(setInteractive:), YES);
        return e;
    }
    return [UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemThinMaterialDark];
}

static BOOL TTGIsButton(UIView *v) {
    if (v.tag == kGlassTag || v.tag == kSelTag) return NO;
    if ([v isKindOfClass:[UIControl class]]) return YES;
    NSString *n = NSStringFromClass([v class]);
    return [n containsString:@"Button"];
}

static BOOL TTGIsSelected(UIView *b) {
    if ([b isKindOfClass:[UIControl class]] && ((UIControl *)b).isSelected) return YES;
    for (NSString *k in @[@"isSelected", @"selected", @"isCurrent"]) {
        SEL s = NSSelectorFromString(k);
        if ([b respondsToSelector:s]) {
            @try { if ([[b valueForKey:k] boolValue]) return YES; } @catch (__unused id e) {}
        }
    }
    return NO;
}

static void TTGCollectButtons(UIView *v, NSMutableArray *out, int d) {
    if (d > 4) return;
    for (UIView *s in v.subviews) {
        if (s.tag == kGlassTag || s.tag == kSelTag) continue;
        if (TTGIsButton(s) && s.bounds.size.width >= 30 && s.bounds.size.width <= 160) [out addObject:s];
        else TTGCollectButtons(s, out, d + 1);
    }
}
static void TTGClearBG(UIView *v) {
    for (UIView *s in v.subviews) {
        if (TTGIsButton(s)) continue;
        if ([s isKindOfClass:[UIImageView class]] && s.bounds.size.width > 200) s.hidden = YES;
        else if (s.bounds.size.width > 200) { s.backgroundColor = UIColor.clearColor; TTGClearBG(s); }
    }
}

static void TTGSetLabelsHidden(UIView *v, BOOL hidden) {
    for (UIView *s in v.subviews) {
        if ([s isKindOfClass:[UILabel class]]) {
            NSString *t = ((UILabel *)s).text;
            // keep badge counters (digits) visible
            BOOL isBadge = t.length && [t rangeOfCharacterFromSet:NSCharacterSet.decimalDigitCharacterSet.invertedSet].location == NSNotFound;
            if (!isBadge) s.alpha = hidden ? 0 : 1;
        } else TTGSetLabelsHidden(s, hidden);
    }
}

// ---------- pushing the feed under the bar ----------
static void TTGExtend(UIView *v, CGFloat barTop, CGFloat bottom, UIView *skip, int depth) {
    if (!v || depth > 9 || v == skip || v.hidden) return;
    UIWindow *w = v.window; if (!w) return;
    for (UIView *s in v.subviews) {
        if (s == skip || s.hidden || [s isKindOfClass:[UITabBar class]]) continue;
        CGRect r = [s convertRect:s.bounds toView:w];
        if (r.size.height > w.bounds.size.height * 0.4 && fabs(CGRectGetMaxY(r) - barTop) < 4) {
            CGRect f = s.frame;
            f.size.height += (bottom - barTop);
            s.frame = f;
        }
        TTGExtend(s, barTop, bottom, skip, depth + 1);
    }
}

%hook UITabBar

- (void)didMoveToWindow {
    %orig;
    BOOL has = NO;
    for (UIGestureRecognizer *g in self.gestureRecognizers) if ([g.name isEqualToString:@"Alaa_TikTok"]) has = YES;
    if (!has) {
        UILongPressGestureRecognizer *lp = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(ttg_longPress:)];
        lp.name = @"Alaa_TikTok";
        lp.minimumPressDuration = 0.9;
        lp.cancelsTouchesInView = NO;
        [self addGestureRecognizer:lp];
    }
    if (!TTGEnabled()) return;
    UITabBarAppearance *ap = [UITabBarAppearance new];
    [ap configureWithTransparentBackground];
    ap.shadowColor = UIColor.clearColor;
    self.standardAppearance = ap;
    if (@available(iOS 15.0, *)) self.scrollEdgeAppearance = ap;
    self.translucent = YES;
    self.backgroundImage = [UIImage new];
    self.shadowImage = [UIImage new];
}

- (void)layoutSubviews {
    %orig;
    [self ttg_applyGlass];
}

%new
- (void)ttg_longPress:(UILongPressGestureRecognizer *)g {
    if (g.state != UIGestureRecognizerStateBegan) return;
    [[[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleMedium] impactOccurred];
    [self ttg_showMenu];
}

%new
- (void)ttg_showMenu {
    TTGShowSettings(self);
}

- (UIView *)hitTest:(CGPoint)p withEvent:(UIEvent *)e {
    NSArray *info = objc_getAssociatedObject(self, &kBtnsKey);
    if (info && TTGEnabled() && !self.hidden && self.alpha > 0.01) {
        CGRect pill = [info[1] CGRectValue];
        if (CGRectContainsPoint(CGRectInset(pill, -4, -8), p)) {
            if (![objc_getAssociatedObject(self, @selector(ttg_tap:)) boolValue]) {
                UITapGestureRecognizer *tg = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(ttg_tap:)];
                tg.cancelsTouchesInView = YES;
                [self addGestureRecognizer:tg];
                objc_setAssociatedObject(self, @selector(ttg_tap:), @YES, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
            }
            return self;
        }
    }
    return %orig;
}

%new
- (void)ttg_tap:(UITapGestureRecognizer *)g {
    NSArray *info = objc_getAssociatedObject(self, &kBtnsKey);
    if (!info) return;
    NSArray<UIView *> *btns = info[0]; CGRect pill = [info[1] CGRectValue];
    CGFloat x = [g locationInView:self].x;
    CGFloat slotW = (pill.size.width - 12) / btns.count;
    NSInteger i = (NSInteger)floor((x - pill.origin.x - 6) / slotW);
    i = MAX(0, MIN((NSInteger)btns.count - 1, i));
    UIView *b = btns[i];
    if ([b isKindOfClass:[UIControl class]]) {
        UIControl *c = (UIControl *)b;
        [c sendActionsForControlEvents:UIControlEventTouchDown];
        [c sendActionsForControlEvents:UIControlEventTouchUpInside];
    } else {
        for (UIGestureRecognizer *r in b.gestureRecognizers) {
            if ([r isKindOfClass:[UITapGestureRecognizer class]]) {
                @try {
                    NSArray *targets = [r valueForKey:@"_targets"];
                    for (id t in targets) {
                        Ivar it = class_getInstanceVariable([t class], "_target");
                        Ivar ia = class_getInstanceVariable([t class], "_action");
                        if (!it || !ia) continue;
                        id target = object_getIvar(t, it);
                        SEL act = *(SEL *)((uint8_t *)(__bridge void *)t + ivar_getOffset(ia));
                        if (target && act) ((void (*)(id, SEL, id))objc_msgSend)(target, act, r);
                    }
                } @catch (__unused id ex) {}
            }
        }
    }
    dispatch_async(dispatch_get_main_queue(), ^{ [self setNeedsLayout]; });
}

%new
- (void)ttg_reopen {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.35 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{ [self ttg_showMenu]; });
}

%new
- (void)ttg_applyGlass {
    UIView *glass0 = [self viewWithTag:kGlassTag];
    if (!TTGEnabled()) { glass0.hidden = YES; [self viewWithTag:kSelTag].hidden = YES; return; }
    glass0.hidden = NO;
    self.backgroundColor = UIColor.clearColor;
    self.clipsToBounds = NO;

    NSMutableArray<UIView *> *buttons = [NSMutableArray array];
    for (UIView *v in self.subviews) {
        if (v.tag == kGlassTag || v.tag == kSelTag) continue;
        if (TTGIsButton(v)) { [buttons addObject:v]; continue; }
        if ([v isKindOfClass:[UIImageView class]] || [v isKindOfClass:[UIVisualEffectView class]]) { v.hidden = YES; continue; }
        NSMutableArray *inner = [NSMutableArray array];
        TTGCollectButtons(v, inner, 0);
        if (inner.count) { [buttons addObjectsFromArray:inner]; v.backgroundColor = UIColor.clearColor; TTGClearBG(v); }
        else if (v.subviews.count == 0) v.hidden = YES;
        else v.backgroundColor = UIColor.clearColor;
    }
    [buttons sortUsingComparator:^NSComparisonResult(UIView *a, UIView *b) {
        CGFloat ax = [a.superview convertPoint:a.center toView:self].x, bx = [b.superview convertPoint:b.center toView:self].x;
        return [@(ax) compare:@(bx)]; }];

    CGFloat W = self.bounds.size.width;
    CGFloat bottomInset = self.window ? self.window.safeAreaInsets.bottom : 0;
    BOOL noLabels = TTGNoLabels();
    CGRect pill;
    if (TTGFloating()) {
        CGFloat h = 64, pw = MIN(W * 0.76, 340);
        CGFloat y = self.bounds.size.height - bottomInset - h + (bottomInset > 0 ? 12 : -6);
        if (y < 0) y = 0;
        pill = CGRectMake((W - pw) / 2, y, pw, h);
    } else {
        pill = self.bounds;
    }

    UIVisualEffectView *glass = (UIVisualEffectView *)glass0;
    if (!glass) {
        glass = [[UIVisualEffectView alloc] initWithEffect:TTGEffect()];
        glass.tag = kGlassTag;
        glass.userInteractionEnabled = NO;
        glass.clipsToBounds = (TTGStyle() != 0 || !NSClassFromString(@"UIGlassEffect"));
        glass.overrideUserInterfaceStyle = (TTGStyle() == 2) ? UIUserInterfaceStyleLight : UIUserInterfaceStyleDark;
        glass.layer.borderColor = [UIColor colorWithWhite:1 alpha:0.14].CGColor;
        [self insertSubview:glass atIndex:0];
    } else if (self.subviews.firstObject != glass) {
        [self sendSubviewToBack:glass];
    }
    glass.frame = pill;
    glass.layer.cornerRadius = TTGFloating() ? pill.size.height / 2 : 0;
    glass.layer.cornerCurve = kCACornerCurveContinuous;
    TTGCapsule(glass, TTGFloating());
    if (TTGStyle() == 0 && NSClassFromString(@"UIGlassEffect")) glass.layer.borderWidth = 0;
    glass.layer.borderWidth = TTGFloating() ? 0.6 : 0;

    // Instagram-style layout: move buttons into the pill with transforms (TikTok re-lays frames, not transforms).
    NSMutableArray *targets = [NSMutableArray array];
    CGFloat slotW = (pill.size.width - 12) / MAX((NSUInteger)1, buttons.count);
    for (NSUInteger i = 0; i < buttons.count; i++) {
        UIView *b = buttons[i];
        CGPoint c = [b.superview convertPoint:b.center toView:self];
        CGPoint t = c;
        if (TTGFloating() && buttons.count >= 3) {
            t = CGPointMake(pill.origin.x + 6 + slotW * (i + 0.5), CGRectGetMidY(pill) + (noLabels ? 8 : 0));
        }
        if (!CGAffineTransformIsIdentity(b.transform)) b.transform = CGAffineTransformIdentity;
        b.layer.sublayerTransform = CATransform3DMakeTranslation(t.x - c.x, t.y - c.y, 0);
        [targets addObject:[NSValue valueWithCGPoint:CGPointMake(t.x, CGRectGetMidY(pill))]];
    }
    for (UIView *b in buttons) TTGSetLabelsHidden(b, noLabels);
    objc_setAssociatedObject(self, &kBtnsKey, (TTGFloating() && buttons.count >= 3) ? @[[buttons copy], [NSValue valueWithCGRect:pill]] : nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

    // highlight capsule behind the selected tab
    UIView *sel = [self viewWithTag:kSelTag];
    if (!sel) {
        Class gc = NSClassFromString(@"UIGlassEffect");
        if (gc && TTGStyle() == 0) {
            id ge = [gc respondsToSelector:@selector(effectWithStyle:)] ? ((id (*)(id, SEL, NSInteger))objc_msgSend)(gc, @selector(effectWithStyle:), 1) : [[gc alloc] init];
            if ([ge respondsToSelector:@selector(setTintColor:)])
                ((void (*)(id, SEL, id))objc_msgSend)(ge, @selector(setTintColor:), [UIColor colorWithWhite:1 alpha:0.10]);
            sel = [[UIVisualEffectView alloc] initWithEffect:ge];
        } else {
            sel = [UIView new];
            sel.backgroundColor = [UIColor colorWithWhite:1 alpha:0.16];
        }
        sel.tag = kSelTag;
        sel.userInteractionEnabled = NO;
        [self insertSubview:sel aboveSubview:glass];
    }
    NSInteger selIdx = -1;
    for (NSUInteger i = 0; i < buttons.count; i++) if (TTGIsSelected(buttons[i])) { selIdx = i; break; }
    if (selIdx < 0 && [self respondsToSelector:@selector(selectedItem)] && self.selectedItem && self.items.count == buttons.count)
        selIdx = [self.items indexOfObject:self.selectedItem];
    if (NO && selIdx >= 0 && selIdx < (NSInteger)targets.count && TTGFloating()) { // highlight removed (1.10)
        CGFloat sw = MIN(slotW + 4, 92), sh = pill.size.height - 10;
        CGPoint tc = [targets[selIdx] CGPointValue];
        sel.frame = CGRectMake(tc.x - sw / 2, pill.origin.y + 5, sw, sh);
        sel.layer.cornerRadius = sh / 2;
        sel.layer.cornerCurve = kCACornerCurveContinuous;
        TTGCapsule(sel, YES);
        sel.hidden = NO;
    } else sel.hidden = YES;
}

%end

static void TTGRefreshIn(UIView *v) {
    if ([v isKindOfClass:[UITabBar class]]) { [v setNeedsLayout]; [v layoutIfNeeded]; return; }
    for (UIView *s in v.subviews) TTGRefreshIn(s);
}
static void TTGRefreshAll(void) {
    for (UIWindow *w in UIApplication.sharedApplication.windows) TTGRefreshIn(w);
}

static BOOL TTGIsVideoView(UIView *v) {
    if ([v.layer isKindOfClass:[AVPlayerLayer class]]) return YES;
    for (CALayer *l in v.layer.sublayers) if ([l isKindOfClass:[AVPlayerLayer class]]) return YES;
    NSString *n = NSStringFromClass(v.class).lowercaseString;
    if ([n containsString:@"interact"] || [n containsString:@"overlay"] || [n containsString:@"control"] || [n containsString:@"mask"]) return NO;
    return [n containsString:@"player"] || [n containsString:@"video"] || [n containsString:@"cover"] || [n containsString:@"render"] || [v isKindOfClass:[UIImageView class]];
}

static void TTGFillFeed(UIView *v, CGFloat barTop, CGFloat fullH, int d) {
    if (d > 16) return;
    for (UIView *s in v.subviews) {
        if (s.hidden) continue;
        CGFloat ph = s.superview.bounds.size.height;
        BOOL parentOK = ph >= fullH - 4 || fabs(ph - barTop) < 4;
        if (fabs(s.frame.size.height - barTop) < 4 && fabs(s.frame.origin.y) < 2 && parentOK && TTGIsVideoView(s)) {
            CGRect f = s.frame; f.size.height = fullH; s.frame = f;
            for (UIView *p = s.superview; p && p != v.window; p = p.superview) {
                if (p.bounds.size.height >= fullH - 4) break;
                p.clipsToBounds = NO;
            }
        }
        TTGFillFeed(s, barTop, fullH, d + 1);
    }
}

%hook AWEFeedSlidingScrollView
- (void)layoutSubviews {
    %orig;
    UIView *me = (UIView *)self;
    UIWindow *w = me.window;
    if (!w || !TTGEnabled() || !TTGUnder()) return;
    CGFloat fullH = w.bounds.size.height;
    CGFloat barTop = fullH - 83;
    for (UIWindow *ww in UIApplication.sharedApplication.windows) {
        UIView *tb = nil; NSMutableArray *st = [NSMutableArray arrayWithObject:ww];
        while (st.count && !tb) { UIView *x = st.lastObject; [st removeLastObject];
            if ([x isKindOfClass:[UITabBar class]] && !x.hidden) tb = x; else [st addObjectsFromArray:x.subviews]; }
        if (tb) { barTop = [tb convertRect:tb.bounds toView:w].origin.y; break; }
    }
    if (fullH - barTop < 20) return;
    TTGFillFeed(me, barTop, fullH, 0);
}
%end

%hook AWENormalModeTabBarController
- (void)viewDidLayoutSubviews {
    %orig;
    UIViewController *vc = (UIViewController *)self;
    if (![vc respondsToSelector:@selector(tabBar)] || !TTGEnabled()) return;
    UITabBar *tb = [(id)vc tabBar];
    UIView *root = vc.view;
    if (!tb || !root.window) return;
    UIView *barHost = tb;
    while (barHost.superview && barHost.superview != root) barHost = barHost.superview;
    barHost.backgroundColor = UIColor.clearColor;
    tb.superview.backgroundColor = UIColor.clearColor;
    if (!TTGUnder()) return;
    UIWindow *w = root.window;
    CGFloat barTop = [tb convertRect:tb.bounds toView:w].origin.y;
    CGFloat bottom = w.bounds.size.height;
    if (bottom - barTop < 20) return;
    TTGExtend(root, barTop, bottom, barHost, 0);
    root.backgroundColor = UIColor.clearColor;
    [root bringSubviewToFront:barHost];
}
%end

// ---- 1.7: stretch the feed player into the area under the floating bar ----
static CGFloat TTGBarH(void) { return 83.0; }
static CGRect TTGStretch(UIView *v, CGRect f) {
    if (!TTGEnabled() || !TTGUnder()) return f;
    CGFloat full = UIScreen.mainScreen.bounds.size.height;
    CGFloat barTop = full - TTGBarH();
    if (fabs(f.size.height - barTop) < 4) f.size.height = full;
    return f;
}
static void TTGUnclip(UIView *v) {
    CGFloat full = UIScreen.mainScreen.bounds.size.height;
    for (UIView *p = v.superview; p; p = p.superview) {
        if (p.bounds.size.height >= full - 4) break;
        if (p.clipsToBounds) p.clipsToBounds = NO;
    }
}

%hook AWEAwemeModel
- (BOOL)progressBarVisible { return TTExtraEnabled(@"tt.extra.progress") ? YES : %orig; }
- (BOOL)progressBarDraggable { return TTExtraEnabled(@"tt.extra.progress") ? YES : %orig; }
%end
%hook AWEPlayVideoPlayerControllerBackgroundView
- (void)setFrame:(CGRect)f { %orig(TTGStretch((UIView *)self, f)); }
- (void)layoutSubviews { %orig; if (TTGEnabled() && TTGUnder()) TTGUnclip((UIView *)self); }
%end
%hook TTKFeedPlayerContainerView
- (void)setFrame:(CGRect)f { %orig(TTGStretch((UIView *)self, f)); }
%end
%hook TTPlayerView
- (void)setFrame:(CGRect)f { %orig(TTGStretch((UIView *)self, f)); }
%end
%hook TTMetalView
- (void)setFrame:(CGRect)f { %orig(TTGStretch((UIView *)self, f)); }
%end
%hook MTKView
- (void)setFrame:(CGRect)f { if ([((UIView *)self).superview isKindOfClass:NSClassFromString(@"TTMetalView")]) %orig(TTGStretch((UIView *)self, f)); else %orig; }
%end
%hook TTKMediaBusinessCoverImageView
- (void)setFrame:(CGRect)f { %orig(TTGStretch((UIView *)self, f)); }
%end
// ---- 1.17: apply action glass on first display and after reuse/state updates ----
static char kActionPendingKey;
static const NSInteger kActionGlassTag = 0x61A57;

// Bounded rolling class/source/reason diagnostics, no customer labels or media.
static void TTGDiagnostic(UIView *view, NSString *reason) {
    static NSMutableOrderedSet *events;
    static BOOL pending;
    if (!events) events = [NSMutableOrderedSet orderedSet];
    NSString *line = [NSString stringWithFormat:@"%@ %@", NSStringFromClass(view.class), reason];
    if ([events containsObject:line]) return;
    [events addObject:line];
    if (events.count > 120) [events removeObjectAtIndex:0];
    if (pending) return;
    pending = YES;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        pending = NO;
        NSString *dir = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
        NSString *text = [@"Alaa_TikTok 1.21 class/source diagnostics\n" stringByAppendingString:[events.array componentsJoinedByString:@"\n"]];
        if (dir) [text writeToFile:[dir stringByAppendingPathComponent:@"alaa_tiktok_debug.txt"] atomically:YES encoding:NSUTF8StringEncoding error:nil];
    });
}
static void TTGActionDebug(UIView *button) { TTGDiagnostic(button, @"glass attached"); }

// TikTok has shipped both aweImageView and imageView for AWEFeedVideoButton.
// Only call known object-valued getters; never use unchecked private ivars/KVC.
static id TTGActionObject(id object, NSString *name) {
    SEL selector = NSSelectorFromString(name);
    if (!object || ![object respondsToSelector:selector]) return nil;
    NSMethodSignature *signature = [object methodSignatureForSelector:selector];
    if (!signature || signature.numberOfArguments != 2 || signature.methodReturnType[0] != '@') return nil;
    @try { return ((id (*)(id, SEL))objc_msgSend)(object, selector); }
    @catch (__unused NSException *exception) { return nil; }
}

static BOOL TTGActionHasCount(UIView *view, NSUInteger depth) {
    if (depth > 4 || view.tag == kActionGlassTag) return NO;
    if ([view isKindOfClass:UILabel.class]) {
        NSString *text = ((UILabel *)view).text;
        return text.length && [text rangeOfCharacterFromSet:NSCharacterSet.decimalDigitCharacterSet].location != NSNotFound;
    }
    for (UIView *child in view.subviews) if (TTGActionHasCount(child, depth + 1)) return YES;
    return NO;
}

static void TTGChildIdentity(UIView *node, NSMutableString *identity, NSUInteger depth) {
    if (depth > 2 || node.tag == kActionGlassTag || [node isKindOfClass:UILabel.class] ||
        [node isKindOfClass:UIVisualEffectView.class]) return;
    if (node.bounds.size.width <= 80 && node.bounds.size.height <= 80) {
        [identity appendFormat:@" %@", NSStringFromClass(node.class)];
        for (NSString *key in @[@"imageNameString", @"accessibilityIdentifier"]) {
            id value = TTGActionObject(node, key);
            if ([value isKindOfClass:NSString.class]) [identity appendFormat:@" %@", value];
        }
    }
    for (UIView *child in node.subviews) TTGChildIdentity(child, identity, depth + 1);
}
static BOOL TTGActionIdentity(UIView *button) {
    if ([button isKindOfClass:UILabel.class] || [button isKindOfClass:UIImageView.class]) return NO;
    NSMutableString *identity = [NSMutableString stringWithString:NSStringFromClass(button.class)];
    for (NSString *key in @[@"imageNameString", @"accessibilityIdentifier", @"accessibilityLabel"]) {
        id value = TTGActionObject(button, key);
        if ([value isKindOfClass:NSString.class]) [identity appendFormat:@" %@", value];
    }
    for (UIView *child in button.subviews) TTGChildIdentity(child, identity, 0);
    NSString *name = [identity stringByFoldingWithOptions:NSDiacriticInsensitiveSearch locale:nil].lowercaseString;
    for (NSString *excluded in @[@"avatar", @"follow", @"music", @"disc", @"download", @"folgen", @"musik", @"متابعة", @"موسيقى"])
        if ([name containsString:excluded]) return NO;
    for (NSString *action in @[@"like", @"digg", @"heart", @"comment", @"collect", @"favorite", @"favourite", @"bookmark", @"save", @"share", @"forward", @"gefallt mir", @"kommentar", @"favoriten", @"teilen", @"speichern", @"اعجاب", @"تعليق", @"مفضلة", @"مشاركة", @"حفظ"])
        if ([name containsString:action]) return YES;
    // The confirmed action class may not populate its image-name getter until
    // highlight. A count within this exact button is a safe initial-display
    // fallback; TTGActionInFeed separately enforces the right-edge feed column.
    return ([button isKindOfClass:NSClassFromString(@"AWEFeedVideoButton")] ||
            [button isKindOfClass:UIControl.class]) && TTGActionHasCount(button, 0);
}

static BOOL TTGActionInFeed(UIView *button) {
    UIWindow *window = button.window;
    if (!window || button.hidden || button.alpha < 0.01 || button.bounds.size.width < 24 || button.bounds.size.width > 100 ||
        button.bounds.size.height < 24 || button.bounds.size.height > 150) return NO;
    CGRect rect = [button convertRect:button.bounds toView:window];
    // Never decorate comments-panel/toolbars/profile actions.
    if ((CGRectGetMidX(rect) > CGRectGetWidth(window.bounds) * 0.30 &&
         CGRectGetMidX(rect) < CGRectGetWidth(window.bounds) * 0.70) ||
        CGRectGetMidY(rect) < 80 || CGRectGetMidY(rect) > CGRectGetHeight(window.bounds) - 85) return NO;
    BOOL feed = NO;
    for (UIView *parent = button.superview; parent; parent = parent.superview) {
        NSString *name = NSStringFromClass(parent.class).lowercaseString;
        if ([parent isKindOfClass:UITabBar.class] || [name containsString:@"commentpanel"] ||
            [name containsString:@"commentsheet"] || [name containsString:@"profile"] || [name containsString:@"settings"]) return NO;
        if ([name containsString:@"feed"] || [name containsString:@"playinteraction"]) feed = YES;
    }
    return feed;
}


static char kGlyphStateKey, kGlyphBusyKey, kRendererOwnerKey, kCommentLayerOwnerKey;
@interface TTGActionGlyph : UIView
@property(nonatomic, weak) UIView *sourceIcon;
@property(nonatomic, strong) NSMapTable<UIView *, NSNumber *> *rendererAlphas;
@property(nonatomic, strong) NSMapTable<CALayer *, NSNumber *> *commentLayerOpacities;
@property(nonatomic) CGFloat savedAlpha;
@property(nonatomic, strong) UIImage *sourceImage;
@property(nonatomic) CGSize sourceSize;
@property(nonatomic) UIViewContentMode sourceMode;
@property(nonatomic) BOOL liked;
@property(nonatomic) CGFloat sourceScale;
@property(nonatomic, strong) CALayer *silhouetteClip;
@property(nonatomic, strong) UIVisualEffectView *glass;
@property(nonatomic, strong) UIImageView *glassMask;
@property(nonatomic, strong) UIImageView *body;
@property(nonatomic, strong) UIImageView *innerShadow;
@property(nonatomic, strong) CAGradientLayer *rim;
@property(nonatomic, strong) CALayer *rimMask;
@end

static UIVisualEffect *TTGGlyphEffect(BOOL liked) {
    Class cls = NSClassFromString(@"UIGlassEffect");
    if (cls && [cls respondsToSelector:@selector(effectWithStyle:)]) {
        id effect = ((id (*)(id, SEL, NSInteger))objc_msgSend)(cls, @selector(effectWithStyle:), 1);
        if ([effect respondsToSelector:@selector(setTintColor:)])
            ((void (*)(id, SEL, id))objc_msgSend)(effect, @selector(setTintColor:),
                liked ? [UIColor colorWithRed:1 green:0.12 blue:0.24 alpha:0.26] : nil);
        return effect;
    }
    return [UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemThinMaterialDark];
}

// Rasterize the image itself, never UIImageView.layer (which may include shadows,
// animations or cached effects). All masks share exact screen-pixel dimensions.
static NSArray<UIImage *> *TTGGlyphMasksWithEdge(UIImage *image, CGSize size, UIViewContentMode mode, CGFloat scale, CGFloat edge) {
    if (!image || size.width <= 0 || size.height <= 0 || scale <= 0) return nil;
    size = CGSizeMake(round(size.width * scale) / scale, round(size.height * scale) / scale);
    CGSize natural = image.size;
    if (natural.width <= 0 || natural.height <= 0) return nil;
    CGRect draw = (CGRect){CGPointZero, size};
    if (mode == UIViewContentModeScaleAspectFit || mode == UIViewContentModeScaleAspectFill) {
        CGFloat x = size.width / natural.width, y = size.height / natural.height;
        CGFloat ratio = mode == UIViewContentModeScaleAspectFit ? MIN(x, y) : MAX(x, y);
        draw.size = CGSizeMake(natural.width * ratio * 1.035, natural.height * ratio * 1.035);
        draw.origin = CGPointMake((size.width - draw.size.width) / 2, (size.height - draw.size.height) / 2);
    } else if (mode != UIViewContentModeScaleToFill && mode != UIViewContentModeRedraw) {
        draw.size = natural;
        draw.origin = CGPointMake((size.width - natural.width) / 2, (size.height - natural.height) / 2);
        if (mode == UIViewContentModeTop || mode == UIViewContentModeTopLeft || mode == UIViewContentModeTopRight) draw.origin.y = 0;
        if (mode == UIViewContentModeBottom || mode == UIViewContentModeBottomLeft || mode == UIViewContentModeBottomRight) draw.origin.y = size.height - natural.height;
        if (mode == UIViewContentModeLeft || mode == UIViewContentModeTopLeft || mode == UIViewContentModeBottomLeft) draw.origin.x = 0;
        if (mode == UIViewContentModeRight || mode == UIViewContentModeTopRight || mode == UIViewContentModeBottomRight) draw.origin.x = size.width - natural.width;
    }
    UIGraphicsBeginImageContextWithOptions(size, NO, scale);
    [[image imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal] drawInRect:draw];
    UIImage *raw = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    if (!raw.CGImage) return nil;
    size_t width = CGImageGetWidth(raw.CGImage), height = CGImageGetHeight(raw.CGImage);
    if (!width || !height || width > 512 || height > 512) return nil;
    NSMutableData *pixels = [NSMutableData dataWithLength:width * height * 4];
    CGColorSpaceRef color = CGColorSpaceCreateDeviceRGB();
    CGContextRef bitmap = CGBitmapContextCreate(pixels.mutableBytes, width, height, 8, width * 4, color,
        kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(color);
    if (!bitmap) return nil;
    CGContextDrawImage(bitmap, CGRectMake(0, 0, width, height), raw.CGImage);
    uint8_t *bytes = pixels.mutableBytes;
    // Discard diffuse shadow alpha, retain a one-screen-pixel antialiased edge.
    // RGB is irrelevant: the result is a clean white silhouette, not a grey icon.
    size_t filledPixels = 0;
    for (size_t i = 0; i < width * height; i++) {
        CGFloat a = bytes[i * 4 + 3] / 255.0;
        a = MAX(0, MIN(1, (a - 0.22) / 0.40));
        uint8_t value = (uint8_t)round(a * 255);
        if (value > 96) filledPixels++;
        bytes[i * 4] = value; bytes[i * 4 + 1] = value;
        bytes[i * 4 + 2] = value; bytes[i * 4 + 3] = value;
    }
    if (filledPixels < 12) { CGContextRelease(bitmap); return nil; }
    CGImageRef clean = CGBitmapContextCreateImage(bitmap);
    CGContextRelease(bitmap);
    if (!clean) return nil;
    UIImage *alpha = [UIImage imageWithCGImage:clean scale:scale orientation:UIImageOrientationUp];
    CGImageRelease(clean);
    // Inner bevel only. Nothing protrudes outside the glyph silhouette.
    CGFloat step = MAX(1, round(edge * scale)) / scale;
    UIGraphicsBeginImageContextWithOptions(size, NO, scale);
    [alpha drawAtPoint:CGPointZero];
    for (NSInteger x = -1; x <= 1; x++) for (NSInteger y = -1; y <= 1; y++)
        if (x || y) [alpha drawAtPoint:CGPointMake(x * step, y * step) blendMode:kCGBlendModeDestinationIn alpha:1];
    UIImage *eroded = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    UIGraphicsBeginImageContextWithOptions(size, NO, scale);
    [alpha drawAtPoint:CGPointZero];
    [eroded drawAtPoint:CGPointZero blendMode:kCGBlendModeDestinationOut alpha:1];
    UIImage *rim = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    UIGraphicsBeginImageContextWithOptions(size, NO, scale);
    [alpha drawAtPoint:CGPointZero];
    [alpha drawAtPoint:CGPointMake(-step, -step) blendMode:kCGBlendModeDestinationOut alpha:1];
    UIImage *inner = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return alpha && rim && inner ? @[alpha, rim, inner] : nil;
}

static NSArray<UIImage *> *TTGGlyphMasks(UIImage *image, CGSize size, UIViewContentMode mode, CGFloat scale) {
    return TTGGlyphMasksWithEdge(image, size, mode, scale, 1.20);
}
static BOOL TTGIsCommentAction(UIView *button) {
    NSMutableString *identity = [NSMutableString stringWithString:NSStringFromClass(button.class)];
    for (NSString *key in @[@"imageNameString", @"accessibilityIdentifier", @"accessibilityLabel"]) {
        id value = TTGActionObject(button, key);
        if ([value isKindOfClass:NSString.class]) [identity appendFormat:@" %@", value];
    }
    for (UIView *child in button.subviews) TTGChildIdentity(child, identity, 0);
    NSString *name = identity.lowercaseString;
    for (NSString *token in @[@"comment", @"kommentar", @"تعليق", @"تعليقات"])
        if ([name containsString:token]) return YES;
    return NO;
}
static NSArray<UIImage *> *TTGCommentMasks(CGSize size, CGFloat scale) {
    if (size.width < 8 || size.height < 8 || scale <= 0) return nil;
    UIImageSymbolConfiguration *config = [UIImageSymbolConfiguration configurationWithPointSize:28
        weight:UIImageSymbolWeightRegular scale:UIImageSymbolScaleMedium];
    UIImage *bubble = [[UIImage systemImageNamed:@"bubble.left.fill" withConfiguration:config]
        imageWithTintColor:UIColor.whiteColor renderingMode:UIImageRenderingModeAlwaysOriginal];
    if (!bubble) return nil;
    CGFloat side = MIN(size.width, size.height);
    CGRect body = CGRectMake((size.width - side * 0.86) / 2, (size.height - side * 0.82) / 2,
        side * 0.86, side * 0.82);
    UIGraphicsBeginImageContextWithOptions(size, NO, scale);
    [bubble drawInRect:body];
    UIImage *outer = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    NSArray<UIImage *> *base = TTGGlyphMasksWithEdge(outer, size, UIViewContentModeScaleToFill, scale, 0.70);
    if (base.count != 3) return nil;
    return base;
}

@implementation TTGActionGlyph
- (instancetype)init {
    self = [super initWithFrame:CGRectZero];
    if (!self) return nil;
    self.tag = kActionGlassTag;
    self.userInteractionEnabled = NO;
    self.isAccessibilityElement = NO;
    self.accessibilityElementsHidden = YES;
    self.backgroundColor = UIColor.clearColor;
    self.clipsToBounds = YES;
    _silhouetteClip = [CALayer layer];
    self.layer.mask = _silhouetteClip;
    _glass = [[UIVisualEffectView alloc] initWithEffect:TTGGlyphEffect(NO)];
    _glass.userInteractionEnabled = NO;
    _glass.clipsToBounds = YES;
    _glassMask = [UIImageView new];
    _glassMask.tintColor = UIColor.whiteColor;
    _glass.maskView = _glassMask;
    [self addSubview:_glass];
    _body = [UIImageView new];
    _body.tintColor = [UIColor colorWithWhite:1 alpha:0.065];
    [self addSubview:_body];
    _innerShadow = [UIImageView new];
    _innerShadow.tintColor = [UIColor colorWithWhite:0 alpha:0.24];
    _innerShadow.layer.shadowColor = UIColor.blackColor.CGColor;
    _innerShadow.layer.shadowOpacity = 0; // no shadow bleed beyond the exact alpha mask
    _innerShadow.clipsToBounds = YES;
    _innerShadow.layer.shadowRadius = 0.6;
    _innerShadow.layer.shadowOffset = CGSizeMake(0.35, 0.35);
    [self addSubview:_innerShadow];
    _rim = [CAGradientLayer layer];
    _rim.startPoint = CGPointMake(0, 0);
    _rim.endPoint = CGPointMake(1, 1);
    _rim.colors = @[(id)[UIColor colorWithWhite:1 alpha:1.0].CGColor,
        (id)[UIColor colorWithWhite:1 alpha:0.95].CGColor,
        (id)[UIColor colorWithRed:0.75 green:0.94 blue:1 alpha:0.88].CGColor,
        (id)[UIColor colorWithWhite:1 alpha:0.48].CGColor,
        (id)[UIColor colorWithWhite:1 alpha:0.88].CGColor];
    _rim.locations = @[@0, @0.32, @0.52, @0.76, @1];
    _rimMask = [CALayer layer];
    _rimMask.contentsScale = UIScreen.mainScreen.scale;
    _rim.mask = _rimMask;
    [self.layer addSublayer:_rim];
    return self;
}
@end

// A like action may draw an image and a sibling Lottie/commerce state renderer.
// Dim paint roots only, never the action host, counters, follow/avatar or gestures.
static void TTGRestoreRenderer(TTGActionGlyph *glyph, UIView *renderer) {
    NSNumber *saved = [glyph.rendererAlphas objectForKey:renderer];
    objc_setAssociatedObject(renderer, &kRendererOwnerKey, nil, OBJC_ASSOCIATION_ASSIGN);
    if (saved && renderer.alpha <= 0.103) renderer.alpha = saved.doubleValue;
    [glyph.rendererAlphas removeObjectForKey:renderer];
}
static void TTGRestoreCommentLayers(TTGActionGlyph *glyph) {
    for (CALayer *layer in glyph.commentLayerOpacities.keyEnumerator.allObjects) {
        NSNumber *saved = [glyph.commentLayerOpacities objectForKey:layer];
        objc_setAssociatedObject(layer, &kCommentLayerOwnerKey, nil, OBJC_ASSOCIATION_ASSIGN);
        [layer removeAnimationForKey:@"opacity"];
        layer.opacity = saved.floatValue;
    }
    [glyph.commentLayerOpacities removeAllObjects];
}
static void TTGHideCommentLayer(CALayer *layer, TTGActionGlyph *glyph, UIView *button) {
    if (!layer || layer == button.layer || layer == glyph.layer) return;
    if (!glyph.commentLayerOpacities) glyph.commentLayerOpacities = [NSMapTable weakToStrongObjectsMapTable];
    if (![glyph.commentLayerOpacities objectForKey:layer])
        [glyph.commentLayerOpacities setObject:@(layer.opacity) forKey:layer];
    NSHashTable *owner = [NSHashTable weakObjectsHashTable]; [owner addObject:button];
    objc_setAssociatedObject(layer, &kCommentLayerOwnerKey, owner, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    [layer removeAnimationForKey:@"opacity"];
    layer.opacity = 0;
}
static void TTGHideCommentDetails(UIView *node, TTGActionGlyph *glyph, UIView *button,
                                  CGRect rect, NSUInteger depth) {
    if (depth > 8 || node == glyph || node.tag == kActionGlassTag ||
        [node isKindOfClass:UIVisualEffectView.class]) return;
    CGRect frame = [node convertRect:node.bounds toView:button];
    // Include dots drawn with UILabel/custom views, not just image/Lottie roots.
    // Only a wholly contained glyph-area view can be hidden, never the count below.
    CGRect region = CGRectInset(rect, -3, -3);
    if (node != button && !CGRectIsEmpty(frame) && CGRectContainsRect(region, frame) &&
        !TTGActionHasCount(node, 0)) {
        if (!glyph.rendererAlphas) glyph.rendererAlphas = [NSMapTable weakToStrongObjectsMapTable];
        if (![glyph.rendererAlphas objectForKey:node])
            [glyph.rendererAlphas setObject:@(node.alpha) forKey:node];
        NSHashTable *owner = [NSHashTable weakObjectsHashTable]; [owner addObject:button];
        objc_setAssociatedObject(node, &kRendererOwnerKey, owner, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        node.alpha = 0;
        TTGHideCommentLayer(node.layer, glyph, button);
        TTGDiagnostic(button, [@"comment detail hidden " stringByAppendingString:NSStringFromClass(node.class)]);
    }
    for (UIView *child in node.subviews) TTGHideCommentDetails(child, glyph, button, rect, depth + 1);
    // Unbacked CALayer siblings do not obey UIImageView alpha hooks.
    for (CALayer *layer in node.layer.sublayers) {
        if ([layer.delegate isKindOfClass:UIView.class]) continue;
        CGRect layerFrame = [node.layer convertRect:layer.bounds fromLayer:layer];
        CGRect inButton = [node convertRect:layerFrame toView:button];
        if (!CGRectIsEmpty(inButton) && CGRectContainsRect(region, inButton))
            TTGHideCommentLayer(layer, glyph, button);
    }
}

static void TTGRestoreGlyph(UIView *button) {
    TTGActionGlyph *glyph = objc_getAssociatedObject(button, &kGlyphStateKey);
    if (!glyph) return;
    TTGRestoreCommentLayers(glyph);
    for (UIView *renderer in glyph.rendererAlphas.keyEnumerator.allObjects) TTGRestoreRenderer(glyph, renderer);
    UIView *icon = glyph.sourceIcon;
    if (icon && fabs(icon.alpha - 0.15) < 0.002) icon.alpha = glyph.savedAlpha;
    [glyph removeFromSuperview];
    objc_setAssociatedObject(button, &kGlyphStateKey, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}
static void TTGCollectRenderers(UIView *node, UIView *button, CGRect rect,
                              NSMutableArray<UIView *> *out, NSUInteger depth) {
    if (depth > 6 || node.tag == kActionGlassTag || [node isKindOfClass:UILabel.class] ||
        [node isKindOfClass:UIVisualEffectView.class]) return;
    NSString *name = NSStringFromClass(node.class).lowercaseString;
    if ([name containsString:@"avatar"] || [name containsString:@"follow"] ||
        [name containsString:@"music"] || [name containsString:@"disc"]) return;
    CGRect frame = [node convertRect:node.bounds toView:button];
    CGRect overlap = CGRectIntersection(rect, frame);
    CGFloat area = frame.size.width * frame.size.height;
    BOOL matches = area > 0 && !CGRectIsNull(overlap) &&
        overlap.size.width * overlap.size.height > MIN(area, rect.size.width * rect.size.height) * 0.55 &&
        node.bounds.size.width >= 8 && node.bounds.size.width <= 80 &&
        node.bounds.size.height >= 8 && node.bounds.size.height <= 80;
    BOOL paint = [node isKindOfClass:UIImageView.class] || [name containsString:@"lottie"] ||
        [name containsString:@"animation"] || [name containsString:@"vector"] ||
        (!node.subviews.count && (node.layer.contents || node.layer.sublayers.count));
    if (node != button && matches && paint && !TTGActionHasCount(node, 0)) {
        [out addObject:node]; return;
    }
    for (UIView *child in node.subviews) TTGCollectRenderers(child, button, rect, out, depth + 1);
}
static void TTGDimRenderers(TTGActionGlyph *glyph, UIView *button, CGRect rect) {
    if (!glyph.rendererAlphas) glyph.rendererAlphas = [NSMapTable weakToStrongObjectsMapTable];
    NSMutableArray *roots = [NSMutableArray array];
    TTGCollectRenderers(button, button, rect, roots, 0);
    if (glyph.sourceIcon && ![roots containsObject:glyph.sourceIcon]) [roots addObject:glyph.sourceIcon];
    // Keep comment detail roots suppressed until overlay teardown, not merely
    // until a later discovery pass chooses a different primary image view.
    for (UIView *old in glyph.rendererAlphas.keyEnumerator.allObjects)
        if (![roots containsObject:old] && !TTGIsCommentAction(button)) TTGRestoreRenderer(glyph, old);
    for (UIView *renderer in roots) {
        if (![glyph.rendererAlphas objectForKey:renderer])
            [glyph.rendererAlphas setObject:@(renderer.alpha) forKey:renderer];
        // Weak box avoids retaining either the source view or the owning action.
        NSHashTable *owner = [NSHashTable weakObjectsHashTable]; [owner addObject:button];
        objc_setAssociatedObject(renderer, &kRendererOwnerKey, owner, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        // Keep native dot artwork out of the plain glass bubble.
        CGFloat limit = TTGIsCommentAction(button) ? 0.0 : 0.10;
        renderer.alpha = MIN(limit, [[glyph.rendererAlphas objectForKey:renderer] doubleValue]);
        TTGDiagnostic(button, [@"paint root dimmed " stringByAppendingString:NSStringFromClass(renderer.class)]);
    }
}

static UIImageView *TTGFindImage(UIView *view, NSUInteger depth) {
    if (depth > 4 || view.hidden || view.tag == kActionGlassTag || [view isKindOfClass:UIVisualEffectView.class]) return nil;
    if ([view isKindOfClass:UIImageView.class] && view.bounds.size.width >= 8 && view.bounds.size.width <= 70 &&
        (((UIImageView *)view).image || ((UIImageView *)view).highlightedImage || ((UIImageView *)view).animationImages.count))
        return (UIImageView *)view;
    for (UIView *child in view.subviews) {
        UIImageView *found = TTGFindImage(child, depth + 1);
        if (found && (found.image || found.highlightedImage || found.animationImages.count)) return found;
    }
    return nil;
}

static UIImage *TTGSymbolForAction(UIView *button) {
    NSMutableString *identity = [NSMutableString stringWithString:NSStringFromClass(button.class)];
    for (NSString *key in @[@"imageNameString", @"accessibilityIdentifier", @"accessibilityLabel"]) {
        id value = TTGActionObject(button, key);
        if ([value isKindOfClass:NSString.class]) [identity appendFormat:@" %@", value];
    }
    for (UIView *child in button.subviews) TTGChildIdentity(child, identity, 0);
    NSString *name = [identity stringByFoldingWithOptions:NSDiacriticInsensitiveSearch locale:nil].lowercaseString;
    NSArray *sets = @[
        @[@"bubble.right.fill", @"comment", @"kommentar", @"تعليق"],
        @[@"bookmark.fill", @"collect", @"favorite", @"favourite", @"bookmark", @"save", @"speichern", @"favoriten", @"مفضلة", @"حفظ"],
        @[@"arrowshape.turn.up.right.fill", @"share", @"forward", @"teilen", @"مشاركة"],
        @[@"heart.fill", @"like", @"digg", @"heart", @"gefallt", @"اعجاب"]
    ];
    for (NSArray *set in sets) for (NSUInteger i = 1; i < set.count; i++)
        if ([name containsString:set[i]]) return [UIImage systemImageNamed:set[0]];
    return nil;
}
static char kSnapshotImageKey, kSnapshotTimeKey;
static UIView *TTGCustomIcon(UIView *view, NSUInteger depth) {
    if (depth > 5 || view.tag == kActionGlassTag || view.hidden || [view isKindOfClass:UILabel.class] ||
        [view isKindOfClass:UIVisualEffectView.class]) return nil;
    NSString *name = NSStringFromClass(view.class).lowercaseString;
    BOOL renderer = [name containsString:@"lottie"] || [name containsString:@"animation"] ||
        [name containsString:@"icon"] || [name containsString:@"vector"];
    if (renderer && view.bounds.size.width >= 12 && view.bounds.size.width <= 64 &&
        view.bounds.size.height >= 12 && view.bounds.size.height <= 64) return view;
    for (UIView *child in view.subviews) {
        UIView *found = TTGCustomIcon(child, depth + 1);
        if (found) return found;
    }
    // Leaf CALayer/CAShapeLayer renderer, never snapshot the action+count container.
    if (depth > 0 && !view.subviews.count && ![view isKindOfClass:UIControl.class] &&
        view.bounds.size.width >= 16 && view.bounds.size.width <= 60 &&
        view.bounds.size.height >= 16 && view.bounds.size.height <= 60 &&
        (view.layer.contents || view.layer.sublayers.count)) return view;
    return nil;
}
static UIImage *TTGSnapshotIcon(UIView *icon) {
    CFTimeInterval now = CACurrentMediaTime();
    UIImage *cached = objc_getAssociatedObject(icon, &kSnapshotImageKey);
    NSNumber *when = objc_getAssociatedObject(icon, &kSnapshotTimeKey);
    if (cached && now - when.doubleValue < 1.0 && CGSizeEqualToSize(cached.size, icon.bounds.size)) return cached;
    CGFloat opacity = icon.layer.opacity;
    UIImage *image = nil;
    [CATransaction begin]; [CATransaction setDisableActions:YES];
    @try {
        icon.layer.opacity = 1;
        UIGraphicsBeginImageContextWithOptions(icon.bounds.size, NO, icon.window.screen.scale ?: UIScreen.mainScreen.scale);
        CGContextRef ctx = UIGraphicsGetCurrentContext();
        CGContextTranslateCTM(ctx, -icon.bounds.origin.x, -icon.bounds.origin.y);
        [icon.layer renderInContext:ctx];
        image = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
    } @catch (__unused NSException *e) {
        if (UIGraphicsGetCurrentContext()) UIGraphicsEndImageContext();
    } @finally { icon.layer.opacity = opacity; [CATransaction commit]; }
    if (image) {
        objc_setAssociatedObject(icon, &kSnapshotImageKey, image, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        objc_setAssociatedObject(icon, &kSnapshotTimeKey, @(now), OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    return image;
}

static void TTGSyncActionGlass(UIView *button) {
    if (!NSThread.isMainThread || [objc_getAssociatedObject(button, &kGlyphBusyKey) boolValue]) return;
    objc_setAssociatedObject(button, &kGlyphBusyKey, @YES, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    @try {
        if (!TTGEnabled() || !button.window || !TTGActionIdentity(button) || !TTGActionInFeed(button)) {
            TTGRestoreGlyph(button);
            return;
        }
        TTGActionGlyph *glyph = objc_getAssociatedObject(button, &kGlyphStateKey);
        UIView *icon = nil;
        UIImage *image = nil;
        UIViewContentMode mode = UIViewContentModeScaleAspectFit;
        for (NSString *key in @[@"aweImageView", @"imageView", @"iconView", @"animationView", @"lottieView", @"containerView"]) {
            id candidate = TTGActionObject(button, key);
            if ([candidate isKindOfClass:UIView.class] && candidate != button &&
                [candidate isDescendantOfView:button] && [candidate tag] != kActionGlassTag) {
                UIView *found = TTGFindImage(candidate, 0) ?: TTGCustomIcon(candidate, 0);
                if (!found && ![candidate isHidden] && ![key isEqualToString:@"containerView"] && !TTGActionHasCount(candidate, 0) &&
                    [candidate bounds].size.width >= 12 && [candidate bounds].size.width <= 64 &&
                    [candidate bounds].size.height >= 12 && [candidate bounds].size.height <= 64) found = candidate;
                if (found) { icon = found; break; }
            }
        }
        if (!icon) icon = TTGFindImage(button, 0) ?: TTGCustomIcon(button, 0);
        if ([icon isKindOfClass:UIImageView.class]) {
            UIImageView *iv = (id)icon;
            image = (iv.highlighted && iv.highlightedImage) ? iv.highlightedImage : iv.image;
            if (!image && iv.animationImages.count) image = iv.animationImages.firstObject;
            mode = iv.contentMode;
        }
        if (!image && [button isKindOfClass:UIButton.class]) image = ((UIButton *)button).currentImage;
        if (!image && icon) {
            image = TTGSnapshotIcon(icon);
            mode = UIViewContentModeScaleToFill;
            TTGDiagnostic(button, [@"snapshot " stringByAppendingString:NSStringFromClass(icon.class)]);
        }
        if (!image && icon) {
            image = TTGSymbolForAction(button);
            mode = UIViewContentModeScaleAspectFit;
            if (image) TTGDiagnostic(button, @"semantic mask for missing state image");
        }
        CGRect rect = icon ? [icon convertRect:icon.bounds toView:button] : CGRectZero;
        if (!icon || !image || rect.size.width < 8 || rect.size.height < 8 ||
            rect.size.width > 80 || rect.size.height > 80 ||
            !isfinite(rect.origin.x) || !isfinite(rect.origin.y)) {
            // Fail OPEN on absent images or transient animation-only renderers.
            // Never dim a native icon if a complete overlay cannot be installed.
            // Retain a last valid attached glyph through transient renderer changes.
            if (glyph && glyph.sourceIcon == icon && glyph.sourceImage) {
                if (glyph.superview != button) [button addSubview:glyph];
                glyph.hidden = NO;
            } else TTGRestoreGlyph(button);
            TTGDiagnostic(button, icon ? @"no usable source/frame; retry visible" : @"no icon renderer found; retry visible");
            return;
        }
        if (glyph.sourceIcon && glyph.sourceIcon != icon) { TTGRestoreGlyph(button); glyph = nil; }
        BOOL created = !glyph;
        if (!glyph) {
            glyph = [TTGActionGlyph new];
            glyph.sourceIcon = icon;
            glyph.savedAlpha = icon.alpha;
        }
        CGFloat scale = button.window.screen.scale ?: UIScreen.mainScreen.scale;
        // Snap once, then use identical point/pixel dimensions for every layer.
        rect = CGRectMake(round(rect.origin.x * scale) / scale, round(rect.origin.y * scale) / scale,
            round(rect.size.width * scale) / scale, round(rect.size.height * scale) / scale);
        BOOL changed = glyph.sourceScale != scale || created || glyph.sourceImage != image || !CGSizeEqualToSize(glyph.sourceSize, rect.size) ||
            glyph.sourceMode != mode;
        if (changed) {
            NSArray<UIImage *> *masks = TTGIsCommentAction(button) ? TTGCommentMasks(rect.size, scale) :
                TTGGlyphMasks(image, rect.size, mode, scale);
            if (masks.count != 3) {
                UIImage *symbol = TTGSymbolForAction(button);
                masks = symbol ? TTGGlyphMasks(symbol, rect.size, UIViewContentModeScaleAspectFit, scale) : nil;
                if (masks.count == 3) TTGDiagnostic(button, @"symbol mask for GPU-only renderer");
                else { TTGDiagnostic(button, @"mask unavailable; retry visible"); if (created) TTGRestoreGlyph(button); return; }
            }
            glyph.glassMask.image = masks[0];
            glyph.silhouetteClip.contents = (id)masks[0].CGImage;
            glyph.silhouetteClip.contentsScale = scale;
            glyph.rimMask.contentsScale = scale;
            glyph.sourceScale = scale;
            glyph.body.image = [masks[0] imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];
            glyph.rimMask.contents = (id)masks[1].CGImage;
            glyph.innerShadow.image = [masks[2] imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];
            glyph.sourceImage = image;
            glyph.sourceSize = rect.size;
            glyph.sourceMode = mode;
        }
        id value = TTGActionObject(button, @"imageNameString");
        NSString *name = [value isKindOfClass:NSString.class] ? [value lowercaseString] : @"";
        NSString *label = (button.accessibilityLabel ?: @"").lowercaseString;
        BOOL heart = [name containsString:@"like"] || [name containsString:@"digg"] || [name containsString:@"heart"] ||
            [label containsString:@"like"] || [label containsString:@"gefällt"] || [label containsString:@"إعجاب"];
        BOOL selected = [button isKindOfClass:UIControl.class] && ((UIControl *)button).selected;
        CGFloat r=0, g=0, b=0, a=0;
        BOOL redTint = [icon.tintColor getRed:&r green:&g blue:&b alpha:&a] && r > 0.65 && r > g * 1.6 && r > b * 1.25;
        BOOL liked = heart && (selected || redTint || [name containsString:@"after"] ||
            ([name containsString:@"selected"] && ![name containsString:@"unselected"]) ||
            ([name containsString:@"liked"] && ![name containsString:@"unliked"]));
        if (created || glyph.liked != liked) {
            glyph.glass.effect = TTGGlyphEffect(liked);
            glyph.body.tintColor = liked ? [UIColor colorWithRed:1 green:0.08 blue:0.20 alpha:0.30] :
                [UIColor colorWithWhite:1 alpha:0.065];
            glyph.liked = liked;
        }
        [CATransaction begin];
        [CATransaction setDisableActions:YES];
        CGRect frame = rect;
        if (!CGRectEqualToRect(glyph.frame, frame)) glyph.frame = frame;
        glyph.silhouetteClip.frame = glyph.bounds;
        glyph.silhouetteClip.contentsGravity = kCAGravityResize;
        glyph.glass.frame = glyph.bounds;
        glyph.glassMask.frame = glyph.bounds;
        glyph.body.frame = glyph.bounds;
        glyph.innerShadow.frame = glyph.bounds;
        glyph.rim.frame = glyph.bounds;
        glyph.rimMask.frame = glyph.bounds;
        glyph.rimMask.contentsGravity = kCAGravityResize;
        [CATransaction commit];
        // Own overlay at the BUTTON level, independent of replaced image layers.
        // Its alpha is always 1 so UIKit's live backdrop stays valid.
        if (glyph.superview != button) [button addSubview:glyph];
        if (button.subviews.lastObject != glyph) [button bringSubviewToFront:glyph];
        glyph.hidden = NO;
        glyph.alpha = 1;
        if (glyph.glass.maskView != glyph.glassMask) glyph.glass.maskView = glyph.glassMask;
        if (glyph.layer.mask != glyph.silhouetteClip) glyph.layer.mask = glyph.silhouetteClip;
        if (!glyph.glass.effect) glyph.glass.effect = TTGGlyphEffect(liked);
        [glyph setNeedsLayout];
        objc_setAssociatedObject(button, &kGlyphStateKey, glyph, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        TTGDimRenderers(glyph, button, rect);
        if (TTGIsCommentAction(button)) TTGHideCommentDetails(button, glyph, button, rect, 0);
        TTGDiagnostic(button, [@"source " stringByAppendingString:NSStringFromClass(icon.class)]);
        TTGActionDebug(button);
    } @finally {
        objc_setAssociatedObject(button, &kGlyphBusyKey, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
}

// Small, bounded first-display retries only. Normal image/state events sync
// immediately, not merely after a tap or after a delayed cooldown.
static void TTGQueueActionGlass(UIView *button) {
    if (!NSThread.isMainThread) {
        __weak UIView *weak = button;
        dispatch_async(dispatch_get_main_queue(), ^{ if (weak) TTGSyncActionGlass(weak); });
        return;
    }
    TTGSyncActionGlass(button);
    if (!button.window || [objc_getAssociatedObject(button, &kActionPendingKey) boolValue]) return;
    objc_setAssociatedObject(button, &kActionPendingKey, @YES, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    __weak UIView *weak = button;
    for (NSNumber *delay in @[@0.0, @0.12, @0.5]) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delay.doubleValue * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            UIView *strong = weak;
            if (!strong) return;
            TTGSyncActionGlass(strong);
            if (delay.doubleValue >= 0.5) objc_setAssociatedObject(strong, &kActionPendingKey, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        });
    }
}

static UIView *TTGOwnerAction(UIView *view) {
    Class cls = NSClassFromString(@"AWEFeedVideoButton");
    for (NSUInteger depth = 0; view && depth < 9; depth++, view = view.superview) {
        if (view.tag == kActionGlassTag || [view isKindOfClass:UIVisualEffectView.class]) return nil;
        if ((cls && [view isKindOfClass:cls]) || (TTGActionInFeed(view) && TTGActionIdentity(view))) return view;
    }
    return nil;
}

static void TTGChangedImage(UIView *view) {
    UIView *button = TTGOwnerAction(view);
    if (button && ![objc_getAssociatedObject(button, &kGlyphBusyKey) boolValue]) TTGQueueActionGlass(button);
}

static CGFloat TTGGuardRendererAlpha(UIView *renderer, CGFloat proposed) {
    NSHashTable *owners = objc_getAssociatedObject(renderer, &kRendererOwnerKey);
    UIView *button = owners.anyObject;
    TTGActionGlyph *glyph = button ? objc_getAssociatedObject(button, &kGlyphStateKey) : nil;
    if (!glyph || glyph.superview != button || glyph.hidden || !TTGEnabled()) return proposed;
    if (![objc_getAssociatedObject(button, &kGlyphBusyKey) boolValue]) {
        [glyph.rendererAlphas setObject:@(proposed) forKey:renderer];
        if (proposed > 0.10) TTGDiagnostic(button, @"late native alpha reset blocked");
    }
    return MIN(proposed, TTGIsCommentAction(button) ? 0.0 : 0.10);
}
%hook CALayer
- (void)setOpacity:(float)opacity {
    NSHashTable *owners = objc_getAssociatedObject(self, &kCommentLayerOwnerKey);
    UIView *button = owners.anyObject;
    TTGActionGlyph *glyph = button ? objc_getAssociatedObject(button, &kGlyphStateKey) : nil;
    if (glyph && glyph.superview == button && !glyph.hidden && TTGEnabled()) {
        if (![objc_getAssociatedObject(button, &kGlyphBusyKey) boolValue])
            [glyph.commentLayerOpacities setObject:@(opacity) forKey:self];
        %orig(0.0f);
    } else { %orig; }
}
- (void)addAnimation:(CAAnimation *)animation forKey:(NSString *)key {
    NSHashTable *owners = objc_getAssociatedObject(self, &kCommentLayerOwnerKey);
    UIView *button = owners.anyObject;
    TTGActionGlyph *glyph = button ? objc_getAssociatedObject(button, &kGlyphStateKey) : nil;
    // Explicit opacity animations override the model opacity in presentation.
    if (glyph && glyph.superview == button && !glyph.hidden && TTGEnabled() &&
        [animation isKindOfClass:CAPropertyAnimation.class] &&
        [((CAPropertyAnimation *)animation).keyPath isEqualToString:@"opacity"]) return;
    %orig;
}

%end

%hook UIView
- (void)setAlpha:(CGFloat)alpha { %orig(TTGGuardRendererAlpha(self, alpha)); }
- (void)setHidden:(BOOL)hidden {
    %orig;
    NSHashTable *owners = objc_getAssociatedObject(self, &kRendererOwnerKey);
    UIView *button = owners.anyObject;
    if (button && ![objc_getAssociatedObject(button, &kGlyphBusyKey) boolValue]) TTGQueueActionGlass(button);
}
- (void)didAddSubview:(UIView *)view {
    %orig;
    if (!self.window || view.tag == kActionGlassTag || [self isKindOfClass:UIVisualEffectView.class]) return;
    UIView *button = TTGOwnerAction(self);
    if (!button || [objc_getAssociatedObject(button, &kGlyphBusyKey) boolValue]) return;
    __weak UIView *weak = button;
    dispatch_async(dispatch_get_main_queue(), ^{ if (weak.window) TTGSyncActionGlass(weak); });
}
%end

%hook AWEFeedVideoButton
- (void)layoutSubviews { %orig; TTGQueueActionGlass((UIView *)self); }
- (void)didMoveToWindow { %orig; TTGQueueActionGlass((UIView *)self); }
- (void)didMoveToSuperview { %orig; TTGQueueActionGlass((UIView *)self); }
- (void)setImageNameString:(NSString *)name { %orig; TTGQueueActionGlass((UIView *)self); }
- (void)setSelected:(BOOL)selected { %orig; TTGQueueActionGlass((UIView *)self); }
- (void)setHighlighted:(BOOL)highlighted { %orig; TTGQueueActionGlass((UIView *)self); }
%end

%hook UIButton
- (void)setImage:(UIImage *)image forState:(UIControlState)state { %orig; TTGChangedImage((UIView *)self); }
%end

%hook UIImageView
- (void)setImage:(UIImage *)image { %orig; TTGChangedImage((UIView *)self); }
- (void)setHighlightedImage:(UIImage *)image { %orig; TTGChangedImage((UIView *)self); }
- (void)setAnimationImages:(NSArray<UIImage *> *)images { %orig; TTGChangedImage((UIView *)self); }
- (void)setHighlightedAnimationImages:(NSArray<UIImage *> *)images { %orig; TTGChangedImage((UIView *)self); }
- (void)setHighlighted:(BOOL)highlighted { %orig; TTGChangedImage((UIView *)self); }
- (void)startAnimating { %orig; TTGChangedImage((UIView *)self); }
- (void)stopAnimating { %orig; TTGChangedImage((UIView *)self); }
- (void)setTintColor:(UIColor *)color { %orig; TTGChangedImage((UIView *)self); }
- (void)setAlpha:(CGFloat)alpha { %orig(TTGGuardRendererAlpha((UIView *)self, alpha)); TTGChangedImage((UIView *)self); }
- (void)setHidden:(BOOL)hidden { %orig; TTGChangedImage((UIView *)self); }
- (void)layoutSubviews { %orig; TTGChangedImage((UIView *)self); }
- (void)didMoveToSuperview { %orig; TTGChangedImage((UIView *)self); }
%end

static BOOL TTGVisible(UIView *view) {
    if (!view.window || view.hidden || view.alpha < 0.01) return NO;
    for (UIView *parent = view.superview; parent; parent = parent.superview)
        if (parent.hidden || parent.alpha < 0.01) return NO;
    CGRect rect = [view convertRect:view.bounds toView:view.window];
    return !CGRectIsEmpty(CGRectIntersection(rect, view.window.bounds));
}
static void TTGPrepareFeedActions(UIView *root) {
    if (!TTGVisible(root)) return;
    NSMutableArray<UIView *> *queue = [NSMutableArray arrayWithObject:root];
    for (NSUInteger i = 0; i < queue.count && i < 650; i++) {
        UIView *view = queue[i];
        if (view.tag == kActionGlassTag || [view isKindOfClass:UIVisualEffectView.class] || !TTGVisible(view)) continue;
        if (TTGActionInFeed(view) && TTGActionIdentity(view)) {
            TTGSyncActionGlass(view);
        } else if (queue.count < 650) [queue addObjectsFromArray:view.subviews];
    }
}

// Weak roots only; no retained cells, global-window traversal or display-rate work.
@interface TTGFeedWatchdog : NSObject
@property(nonatomic, strong) NSHashTable<UIView *> *roots;
@property(nonatomic, strong) NSTimer *timer;
+ (instancetype)shared;
- (void)addRoot:(UIView *)root;
- (void)tick;
@end
@implementation TTGFeedWatchdog
+ (instancetype)shared {
    static TTGFeedWatchdog *watch;
    static dispatch_once_t once;
    dispatch_once(&once, ^{ watch = [TTGFeedWatchdog new]; watch.roots = [NSHashTable weakObjectsHashTable]; });
    return watch;
}
- (void)addRoot:(UIView *)root {
    if (!root) return;
    [self.roots addObject:root];
    if (!self.timer) {
        self.timer = [NSTimer timerWithTimeInterval:0.5 target:self selector:@selector(tick) userInfo:nil repeats:YES];
        [[NSRunLoop mainRunLoop] addTimer:self.timer forMode:NSRunLoopCommonModes];
    }
    TTGPrepareFeedActions(root);
}
- (void)tick {
    if (!self.roots.allObjects.count) { [self.timer invalidate]; self.timer = nil; return; }
    if (UIApplication.sharedApplication.applicationState != UIApplicationStateActive || !TTGEnabled()) return;
    UIView *best = nil;
    CGFloat bestArea = 0;
    for (UIView *root in self.roots.allObjects) {
        if (!TTGVisible(root)) continue;
        // A visible cell has priority over its scroll container.
        CGRect r = [root convertRect:root.bounds toView:root.window];
        CGRect visible = CGRectIntersection(r, root.window.bounds);
        CGFloat area = visible.size.width * visible.size.height;
        if ([root isKindOfClass:UIScrollView.class]) area *= 0.1;
        if (area > bestArea) { best = root; bestArea = area; }
    }
    if (best) TTGPrepareFeedActions(best);
}
@end
static BOOL TTGFeedRoot(UIView *view) {
    for (UIView *p = view; p; p = p.superview) {
        NSString *name = NSStringFromClass(p.class).lowercaseString;
        if ([name containsString:@"commentpanel"] || [name containsString:@"profile"]) return NO;
        if ([name containsString:@"feed"] || [name containsString:@"playinteraction"]) return YES;
    }
    return NO;
}
static void TT122Track(UIViewController *c);
static void TT122Recover(void);
%hook AWEFeedCellViewController
- (void)viewDidLayoutSubviews { %orig; UIViewController *c = (id)self; if (c.isViewLoaded) [[TTGFeedWatchdog shared] addRoot:c.view]; TT122Track(c); }
- (void)viewDidAppear:(BOOL)animated { %orig; UIViewController *c = (id)self; if (c.isViewLoaded) [[TTGFeedWatchdog shared] addRoot:c.view]; TT122Track(c); }
- (void)setModel:(id)model { %orig; UIViewController *c = (id)self; if (c.isViewLoaded) [[TTGFeedWatchdog shared] addRoot:c.view]; TT122Track(c); }
%end
%hook UICollectionViewCell
- (void)didMoveToWindow { %orig; UIView *v = (id)self; if (v.window && TTGFeedRoot(v)) { [[TTGFeedWatchdog shared] addRoot:v]; TT122Recover(); } }
- (void)prepareForReuse { %orig; UIView *v = (id)self; if (v.window && TTGFeedRoot(v)) { TTGPrepareFeedActions(v); TT122Recover(); } }
%end
%hook UITableViewCell
- (void)didMoveToWindow { %orig; UIView *v = (id)self; if (v.window && TTGFeedRoot(v)) { [[TTGFeedWatchdog shared] addRoot:v]; TT122Recover(); } }
%end
// Inherited callbacks cover photo/ad variants without guessing their private names.
%hook UIScrollView
- (void)didMoveToWindow { %orig; UIView *v = (id)self; if (v.window && TTGFeedRoot(v)) { [[TTGFeedWatchdog shared] addRoot:v]; TT122Recover(); } }
- (void)setContentOffset:(CGPoint)offset {
    %orig;
    if (!self.dragging && !self.decelerating && TTGFeedRoot((id)self)) [[TTGFeedWatchdog shared] addRoot:(id)self];
}
%end

#include "PhaseOne.inc"
#include "LaterFeatures.inc"
#include "StoryTools.inc"
%hook AWEAwemeModel
- (id)initWithDictionary:(id)dictionary error:(id *)error {
    id result = %orig;
    // Inspect the initialized result, which may differ from self.
    if (result && TTExtraEnabled(@"tt.extra.noads") && TTExtraBool(result,NSSelectorFromString(@"isAds"))) return nil;
    return result;
}
%end

static void TT122Track(UIViewController *c) { [[TT122Hub shared] track:c]; }
static void TT122Recover(void) {
    static BOOL pending=NO;if(pending)return;pending=YES;
    dispatch_async(dispatch_get_main_queue(), ^{pending=NO;[[TT122Hub shared] recoverFeed];[[TT122Hub shared] refresh];});
}

%hook AWEFeedCellViewController
- (void)viewWillAppear:(BOOL)animated { %orig; dispatch_async(dispatch_get_main_queue(), ^{ [[TT122Hub shared] track:(id)self]; }); }
- (void)viewDidDisappear:(BOOL)animated { %orig; [[TT122Hub shared] closeMenu];TT122Recover(); }
%end
%hook AWEFeedViewTemplateCell
- (void)configWithModel:(id)model { %orig; [[TTStoryTools shared] attach:(id)self]; }
- (void)configureWithModel:(id)model { %orig; [[TTStoryTools shared] attach:(id)self]; }
%end
%hook TTKStoryDetailTableViewCell
- (void)configWithModel:(id)model { %orig; [[TTStoryTools shared] attach:(id)self]; }
- (void)configureWithModel:(id)model { %orig; [[TTStoryTools shared] attach:(id)self]; }
%end
%hook AWEAwemeModel
- (BOOL)progressBarVisible { return TTExtraEnabled(@"tt.extra.progress") ? YES : %orig; }
- (BOOL)progressBarDraggable { return TTExtraEnabled(@"tt.extra.progress") ? YES : %orig; }
%end
%hook AWEPlayVideoPlayerController
- (void)playerWillLoopPlaying:(id)player {
    if (TTExtraEnabled(@"tt.extra.autoScroll") && TTExtraScrollNext(self)) return;
    %orig;
}
%end
// A custom, real-material sheet; no action-sheet emoji or developer-copy action.
@interface TTGSettingsController : UIViewController
@property(nonatomic, weak) UITabBar *bar;
@property(nonatomic, strong) UIVisualEffectView *panel;
@property(nonatomic, strong) UIView *shade;
@property(nonatomic, strong) UIStackView *stack;
@property(nonatomic, strong) NSLayoutConstraint *panelHeight;
@property(nonatomic, strong) NSLayoutConstraint *panelCenter;
@end
@implementation TTGSettingsController
- (UILabel *)label:(NSString *)text size:(CGFloat)size weight:(UIFontWeight)weight {
    UILabel *label = [UILabel new];
    label.text = text; label.font = [UIFont systemFontOfSize:size weight:weight];
    label.textColor = UIColor.whiteColor; label.numberOfLines = 0;
    label.textAlignment = NSTextAlignmentLeft;
    label.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    return label;
}
- (UIView *)row:(NSString *)title symbol:(NSString *)symbol key:(NSString *)key defaultValue:(BOOL)value tag:(NSInteger)tag {
    UIView *row = [UIView new];
    row.backgroundColor = [UIColor colorWithWhite:1 alpha:0.045];
    row.layer.cornerRadius = 18; row.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    UIImageView *icon = [[UIImageView alloc] initWithImage:[UIImage systemImageNamed:symbol]];
    icon.tintColor = [UIColor colorWithRed:0.82 green:0.93 blue:1 alpha:1];
    icon.contentMode = UIViewContentModeScaleAspectFit;
    UILabel *label = [self label:title size:15 weight:UIFontWeightMedium];
    UISwitch *toggle = [UISwitch new]; toggle.on = PB(key, value); toggle.tag = tag;
    toggle.accessibilityLabel = title;
    toggle.onTintColor = [UIColor colorWithRed:0.78 green:0.70 blue:0.51 alpha:1];
    [toggle addTarget:self action:@selector(changed:) forControlEvents:UIControlEventValueChanged];
    for (UIView *view in @[icon, label, toggle]) { view.translatesAutoresizingMaskIntoConstraints = NO; [row addSubview:view]; }
    [NSLayoutConstraint activateConstraints:@[
        [row.heightAnchor constraintGreaterThanOrEqualToConstant:62],
        [icon.leftAnchor constraintEqualToAnchor:row.leftAnchor constant:16],
        [icon.centerYAnchor constraintEqualToAnchor:row.centerYAnchor],
        [icon.widthAnchor constraintEqualToConstant:24], [icon.heightAnchor constraintEqualToConstant:24],
        [toggle.rightAnchor constraintEqualToAnchor:row.rightAnchor constant:-14],
        [toggle.centerYAnchor constraintEqualToAnchor:row.centerYAnchor],
        [label.leftAnchor constraintEqualToAnchor:icon.rightAnchor constant:12],
        [label.rightAnchor constraintEqualToAnchor:toggle.leftAnchor constant:-12],
        [label.topAnchor constraintEqualToAnchor:row.topAnchor constant:12],
        [label.bottomAnchor constraintEqualToAnchor:row.bottomAnchor constant:-12]
    ]];
    return row;
}
- (void)loadView {
    self.view = [UIView new]; self.view.backgroundColor = UIColor.clearColor;
    self.view.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    self.view.accessibilityViewIsModal = YES;
    self.overrideUserInterfaceStyle = UIUserInterfaceStyleDark;
    UIButton *shade = [UIButton new]; shade.backgroundColor = [UIColor colorWithWhite:0 alpha:0.36];
    [shade addTarget:self action:@selector(close) forControlEvents:UIControlEventTouchUpInside];
    shade.accessibilityLabel = @"إغلاق الإعدادات";
    self.shade = shade; shade.frame = self.view.bounds;
    shade.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.view addSubview:shade];
    self.panel = [[UIVisualEffectView alloc] initWithEffect:[UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemThickMaterialDark]];
    self.panel.translatesAutoresizingMaskIntoConstraints = NO;
    self.panel.layer.cornerRadius = 32;
    self.panel.layer.cornerCurve = kCACornerCurveContinuous;
    self.panel.clipsToBounds = YES;
    self.panel.layer.borderWidth = 0.8;
    self.panel.layer.borderColor = [UIColor colorWithWhite:1 alpha:0.65].CGColor;
    self.panel.contentView.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    self.panel.contentView.backgroundColor = [UIColor colorWithWhite:0.015 alpha:0.55];
    self.panel.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    [self.view addSubview:self.panel];
    UIScrollView *scroll = [UIScrollView new]; scroll.translatesAutoresizingMaskIntoConstraints = NO;
    scroll.alwaysBounceVertical = NO; scroll.showsVerticalScrollIndicator = NO;
    scroll.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    [self.panel.contentView addSubview:scroll];
    self.stack = [UIStackView new]; self.stack.translatesAutoresizingMaskIntoConstraints = NO;
    self.stack.axis = UILayoutConstraintAxisVertical; self.stack.spacing = 10;
    self.stack.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    [scroll addSubview:self.stack];
    UIView *header = [UIView new];
    UIImage *app = nil;
    NSDictionary *icons = [NSBundle.mainBundle objectForInfoDictionaryKey:@"CFBundleIcons"];
    NSString *filename = [icons[@"CFBundlePrimaryIcon"][@"CFBundleIconFiles"] lastObject];
    if ([filename isKindOfClass:NSString.class]) app = [UIImage imageNamed:filename];
    UIImageView *image = [[UIImageView alloc] initWithImage:app ?: [UIImage systemImageNamed:@"play.square.stack.fill"]];
    image.tintColor = UIColor.whiteColor; image.layer.cornerRadius = 15; image.clipsToBounds = YES;
    image.contentMode = UIViewContentModeScaleAspectFit;
    UILabel *title = [self label:@"Alaa_TikTok" size:24 weight:UIFontWeightBold];
    title.textAlignment = NSTextAlignmentLeft;
    UILabel *subtitle = [self label:@"إعدادات الزجاج" size:13 weight:UIFontWeightMedium];
    subtitle.textAlignment = NSTextAlignmentLeft; subtitle.textColor = [UIColor colorWithWhite:1 alpha:0.64];
    UIButton *close = [UIButton buttonWithType:UIButtonTypeSystem];
    [close setImage:[UIImage systemImageNamed:@"xmark.circle.fill"] forState:UIControlStateNormal];
    close.tintColor = [UIColor colorWithWhite:1 alpha:0.75]; close.accessibilityLabel = @"إغلاق";
    [close addTarget:self action:@selector(close) forControlEvents:UIControlEventTouchUpInside];
    for (UIView *view in @[image,title,subtitle,close]) { view.translatesAutoresizingMaskIntoConstraints=NO; [header addSubview:view]; }
    [NSLayoutConstraint activateConstraints:@[
        [header.heightAnchor constraintEqualToConstant:78],
        [image.leftAnchor constraintEqualToAnchor:header.leftAnchor constant:2],
        [image.centerYAnchor constraintEqualToAnchor:header.centerYAnchor],
        [image.widthAnchor constraintEqualToConstant:52], [image.heightAnchor constraintEqualToConstant:52],
        [title.leftAnchor constraintEqualToAnchor:image.rightAnchor constant:12],
        [title.topAnchor constraintEqualToAnchor:header.topAnchor constant:12],
        [title.rightAnchor constraintLessThanOrEqualToAnchor:close.leftAnchor constant:-8],
        [subtitle.leftAnchor constraintEqualToAnchor:title.leftAnchor],
        [subtitle.topAnchor constraintEqualToAnchor:title.bottomAnchor constant:4],
        [close.rightAnchor constraintEqualToAnchor:header.rightAnchor],
        [close.centerYAnchor constraintEqualToAnchor:header.centerYAnchor],
        [close.widthAnchor constraintEqualToConstant:38], [close.heightAnchor constraintEqualToConstant:44]
    ]];
    [self.stack addArrangedSubview:header];
    [self.stack addArrangedSubview:[self row:@"الشريط الزجاجي" symbol:@"rectangle.bottomthird.inset.filled" key:kPrefEnabled defaultValue:YES tag:0]];
    [self.stack addArrangedSubview:[self row:@"كبسولة عائمة" symbol:@"capsule" key:kPrefFloating defaultValue:YES tag:1]];
    [self.stack addArrangedSubview:[self row:@"الفيديو تحت الشريط" symbol:@"rectangle.expand.vertical" key:kPrefUnder defaultValue:YES tag:2]];
    [self.stack addArrangedSubview:[self row:@"إخفاء أسماء الأزرار" symbol:@"textformat" key:kPrefLabels defaultValue:YES tag:3]];
    NSArray *featureTitles = @[@"زر التنزيل العائم",@"زر إخفاء العناصر",@"حلقة تقدم التنزيل",@"تنزيل عادي",@"تنزيل HD",@"أعلى جودة متاحة",@"تنزيل الصوت (ملفات)",@"قائمة النسخ"];
    NSArray *featureSymbols = @[@"arrow.down.circle",@"eye",@"circle.dotted",@"arrow.down",@"h.circle",@"sparkles",@"waveform",@"doc.on.doc"];
    NSArray *featureKeys = @[@"tt122.download",@"tt122.eye",@"tt122.progress",@"tt122.standard",@"tt122.hd",@"tt122.highest",@"tt122.audio",@"tt122.copy"];
    for (NSInteger i=0;i<(NSInteger)featureKeys.count;i++)
        [self.stack addArrangedSubview:[self row:featureTitles[i] symbol:featureSymbols[i] key:featureKeys[i] defaultValue:YES tag:100+i]];
    [self.stack addArrangedSubview:[self row:@"إخفاء إعلانات الخلاصة" symbol:@"eye.slash" key:@"tt.extra.noads" defaultValue:NO tag:200]];
    [self.stack addArrangedSubview:[self row:@"استخدام رابط التشغيل (قد يكون بلا علامة)" symbol:@"play.rectangle" key:@"tt.extra.playbackAsset" defaultValue:NO tag:201]];
    [self.stack addArrangedSubview:[self row:@"حفظ القصة بالضغط المطوّل" symbol:@"arrow.down.circle" key:@"tt.extra.storySave" defaultValue:NO tag:202]];
    [self.stack addArrangedSubview:[self row:@"الانتقال تلقائياً بعد نهاية الفيديو" symbol:@"arrow.down.to.line" key:@"tt.extra.autoScroll" defaultValue:NO tag:203]];
    [self.stack addArrangedSubview:[self row:@"إظهار شريط تقدم الفيديو دائماً" symbol:@"slider.horizontal.3" key:@"tt.extra.progress" defaultValue:NO tag:204]];
    UILabel *styleLabel = [self label:@"نمط الشريط" size:13 weight:UIFontWeightSemibold];
    styleLabel.textColor = [UIColor colorWithWhite:1 alpha:0.68];
    [self.stack addArrangedSubview:styleLabel];
    UISegmentedControl *styles = [[UISegmentedControl alloc] initWithItems:@[@"زجاج", @"داكن", @"فاتح"]];
    styles.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    styles.selectedSegmentIndex = MAX(0, MIN(2, TTGStyle()));
    styles.accessibilityLabel = @"نمط الشريط";
    [styles.heightAnchor constraintEqualToConstant:42].active = YES;
    [styles addTarget:self action:@selector(styleChanged:) forControlEvents:UIControlEventValueChanged];
    styles.backgroundColor = [UIColor colorWithWhite:1 alpha:0.08];
    styles.selectedSegmentTintColor = [UIColor colorWithRed:0.80 green:0.74 blue:0.60 alpha:0.32];
    [styles setTitleTextAttributes:@{NSForegroundColorAttributeName:UIColor.whiteColor,
        NSFontAttributeName:[UIFont systemFontOfSize:14 weight:UIFontWeightSemibold]} forState:UIControlStateNormal];
    styles.layer.cornerRadius = 12; styles.clipsToBounds = YES;
    styles.apportionsSegmentWidthsByContent = NO;
    [self.stack addArrangedSubview:styles];
    UILabel *note = [self label:@"الإطفاء الكامل بيتطبّق بعد ما تسكّر TikTok وترجع تفتحه." size:12 weight:UIFontWeightRegular];
    note.textColor = [UIColor colorWithWhite:1 alpha:0.52];
    UIView *footer = [UIView new]; footer.backgroundColor = [UIColor colorWithWhite:0 alpha:0.15];
    footer.layer.cornerRadius = 14; note.translatesAutoresizingMaskIntoConstraints = NO;
    [footer addSubview:note];
    [NSLayoutConstraint activateConstraints:@[
        [note.leftAnchor constraintEqualToAnchor:footer.leftAnchor constant:12],
        [note.rightAnchor constraintEqualToAnchor:footer.rightAnchor constant:-12],
        [note.topAnchor constraintEqualToAnchor:footer.topAnchor constant:14],
        [note.bottomAnchor constraintEqualToAnchor:footer.bottomAnchor constant:-14]
    ]];
    [self.stack setCustomSpacing:16 afterView:styles];
    [self.stack addArrangedSubview:footer];
    UILayoutGuide *safe = self.view.safeAreaLayoutGuide;
    NSLayoutConstraint *height = [self.panel.heightAnchor constraintEqualToConstant:550];
    height.priority = UILayoutPriorityDefaultHigh;
    self.panelHeight = height;
    NSLayoutConstraint *width = [self.panel.widthAnchor constraintEqualToAnchor:safe.widthAnchor constant:-28];
    width.priority = UILayoutPriorityDefaultHigh;
    self.panelCenter = [self.panel.centerYAnchor constraintEqualToAnchor:self.view.topAnchor constant:300];
    self.panelCenter.priority = UILayoutPriorityDefaultHigh;
    [NSLayoutConstraint activateConstraints:@[
        self.panelCenter,
        [self.panel.centerXAnchor constraintEqualToAnchor:self.view.centerXAnchor],
        [self.panel.widthAnchor constraintLessThanOrEqualToConstant:430],
        [self.panel.leftAnchor constraintGreaterThanOrEqualToAnchor:safe.leftAnchor constant:14],
        [self.panel.rightAnchor constraintLessThanOrEqualToAnchor:safe.rightAnchor constant:-14],
        width,
        [self.panel.bottomAnchor constraintLessThanOrEqualToAnchor:safe.bottomAnchor constant:-92],
        [self.panel.topAnchor constraintGreaterThanOrEqualToAnchor:safe.topAnchor constant:12],
        height,
        [scroll.leftAnchor constraintEqualToAnchor:self.panel.contentView.leftAnchor],
        [scroll.rightAnchor constraintEqualToAnchor:self.panel.contentView.rightAnchor],
        [scroll.topAnchor constraintEqualToAnchor:self.panel.contentView.topAnchor],
        [scroll.bottomAnchor constraintEqualToAnchor:self.panel.contentView.bottomAnchor],
        [self.stack.leftAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.leftAnchor constant:20],
        [self.stack.rightAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.rightAnchor constant:-20],
        [self.stack.topAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.topAnchor constant:10],
        [self.stack.bottomAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.bottomAnchor constant:-22],
        [self.stack.widthAnchor constraintEqualToAnchor:scroll.frameLayoutGuide.widthAnchor constant:-40]
    ]];

}
- (void)forceLTR:(UIView *)view {
    view.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    for (UIView *child in view.subviews) [self forceLTR:child];
}
- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    [self forceLTR:self.view];
    CGFloat width = MAX(200, self.panel.bounds.size.width - 40);
    CGFloat natural = [self.stack systemLayoutSizeFittingSize:CGSizeMake(width, UILayoutFittingCompressedSize.height)
        withHorizontalFittingPriority:UILayoutPriorityRequired verticalFittingPriority:UILayoutPriorityFittingSizeLevel].height + 32;
    CGFloat top = self.view.safeAreaInsets.top + 12;
    CGFloat bottom = self.view.bounds.size.height - self.view.safeAreaInsets.bottom - 92;
    UIView *glass = [self.bar viewWithTag:kGlassTag];
    if (glass.window && glass.bounds.size.height > 0) {
        CGRect barFrame = [glass convertRect:glass.bounds toView:self.view];
        if (barFrame.origin.y > top + 150) bottom = MIN(bottom, barFrame.origin.y - 16);
    }
    CGFloat available = MAX(120, bottom - top);
    CGFloat desired = MIN(MAX(180, natural), available);
    if (fabs(self.panelHeight.constant - desired) > 0.5) self.panelHeight.constant = desired;
    CGFloat center = top + available / 2;
    if (fabs(self.panelCenter.constant - center) > 0.5) self.panelCenter.constant = center;
}
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    self.panel.transform = CGAffineTransformMakeTranslation(0, 45);
    self.panel.alpha = 0; self.shade.alpha = 0;
    [UIView animateWithDuration:UIAccessibilityIsReduceMotionEnabled() ? 0.15 : 0.55 delay:0
        usingSpringWithDamping:0.84 initialSpringVelocity:0.3 options:UIViewAnimationOptionBeginFromCurrentState
        animations:^{ self.panel.transform = CGAffineTransformIdentity; self.panel.alpha = 1; self.shade.alpha = 1; } completion:nil];
}
- (void)changed:(UISwitch *)sender {
    if (sender.tag == 204) { [D() setBool:sender.on forKey:@"tt.extra.progress"]; return; }
    if (sender.tag == 203) { [D() setBool:sender.on forKey:@"tt.extra.autoScroll"]; return; }
    if (sender.tag == 202) { [D() setBool:sender.on forKey:@"tt.extra.storySave"]; return; }
    if (sender.tag == 201) { [D() setBool:sender.on forKey:@"tt.extra.playbackAsset"]; return; }
    if (sender.tag == 200) { [D() setBool:sender.on forKey:@"tt.extra.noads"]; return; }
    if (sender.tag >= 100 && sender.tag < 108) {
        NSArray *features = @[@"tt122.download",@"tt122.eye",@"tt122.progress",@"tt122.standard",@"tt122.hd",@"tt122.highest",@"tt122.audio",@"tt122.copy"];
        [D() setBool:sender.on forKey:features[sender.tag-100]];
        [[TT122Hub shared] refresh]; return;
    }
    NSArray *keys = @[kPrefEnabled,kPrefFloating,kPrefUnder,kPrefLabels];
    if (sender.tag < 0 || sender.tag >= (NSInteger)keys.count) return;
    [D() setBool:sender.on forKey:keys[sender.tag]];
    TTGRefreshAll();
}
- (void)styleChanged:(UISegmentedControl *)sender {
    [D() setInteger:sender.selectedSegmentIndex forKey:kPrefStyle];
    [[self.bar viewWithTag:kGlassTag] removeFromSuperview];
    [[self.bar viewWithTag:kSelTag] removeFromSuperview];
    TTGRefreshAll();
}
- (void)close {
    [UIView animateWithDuration:0.18 animations:^{ self.panel.alpha = 0; self.shade.alpha = 0;
        self.panel.transform = CGAffineTransformMakeTranslation(0, 24); }
        completion:^(BOOL finished) { [self dismissViewControllerAnimated:NO completion:nil]; }];
}
@end
static void TTGShowSettings(UITabBar *bar) {
    UIViewController *top = bar.window.rootViewController;
    while (top.presentedViewController) top = top.presentedViewController;
    if (!top || [top isKindOfClass:TTGSettingsController.class]) return;
    TTGSettingsController *controller = [TTGSettingsController new]; controller.bar = bar;
    controller.modalPresentationStyle = UIModalPresentationOverFullScreen;
    [top presentViewController:controller animated:NO completion:nil];
}

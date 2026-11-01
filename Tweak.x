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
    UIViewController *top = self.window.rootViewController;
    while (top.presentedViewController) top = top.presentedViewController;
    if (!top) return;
    NSArray *styles = @[@"Liquid Glass", @"ضبابي داكن", @"ضبابي فاتح"];
    UIAlertController *ac = [UIAlertController alertControllerWithTitle:@"Alaa_TikTok"
        message:@"إعدادات الشريط\n(الإطفاء الكامل بيتطبّق بعد ما تسكّر TikTok وترجع تفتحه)" preferredStyle:UIAlertControllerStyleActionSheet];
    __weak UITabBar *ws = self;
    void (^toggle)(NSString *, NSString *, BOOL) = ^(NSString *title, NSString *key, BOOL def) {
        BOOL on = PB(key, def);
        [ac addAction:[UIAlertAction actionWithTitle:[NSString stringWithFormat:@"%@ %@", on ? @"✅" : @"⬜️", title]
            style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
                [D() setBool:!on forKey:key]; TTGRefreshAll(); [ws ttg_reopen]; }]];
    };
    toggle(@"الشريط الزجاجي", kPrefEnabled, YES);
    toggle(@"كبسولة عائمة (متل انستا)", kPrefFloating, YES);
    toggle(@"الفيديو تحت الشريط", kPrefUnder, YES);
    toggle(@"إخفاء الأسماء (أيقونات بس)", kPrefLabels, YES);
    [ac addAction:[UIAlertAction actionWithTitle:[NSString stringWithFormat:@"🎨 النمط: %@", styles[MAX(0, MIN(2, TTGStyle()))]]
        style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
            [D() setInteger:(TTGStyle() + 1) % 3 forKey:kPrefStyle];
            [[ws viewWithTag:kGlassTag] removeFromSuperview];
            TTGRefreshAll(); [ws ttg_reopen]; }]];
    [ac addAction:[UIAlertAction actionWithTitle:@"📋 نسخ هيكل الواجهة (للمطوّر)" style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
        NSMutableString *out = [NSMutableString string];
        UIWindow *w = ws.window;
        __block void (^dump)(UIView *, int);
        __block __weak void (^wdump)(UIView *, int);
        wdump = dump = ^(UIView *v, int d) {
            if (d > 30 || out.length > 12000) return;
            CGRect r = [v convertRect:v.bounds toView:w];
            BOOL inBar = [v isDescendantOfView:ws];
            if (r.size.height >= 300 || inBar || d < 3 || (CGRectGetMaxY(r) > w.bounds.size.height - 220 && r.size.height >= 18 && r.size.width >= 120))
                [out appendFormat:@"%@%@ %.0f,%.0f %.0fx%.0f%@\n", [@"" stringByPaddingToLength:d withString:@" " startingAtIndex:0],
                    NSStringFromClass(v.class), r.origin.x, r.origin.y, r.size.width, r.size.height, v.hidden ? @" H" : @""];
            for (UIView *s in v.subviews) wdump(s, d + 1);
        };
        dump(w, 0);
        [out appendFormat:@"tabbar %@\n", NSStringFromCGRect([ws convertRect:ws.bounds toView:w])];
        UIPasteboard.generalPasteboard.string = out;
    }]];
    [ac addAction:[UIAlertAction actionWithTitle:@"تم" style:UIAlertActionStyleCancel handler:nil]];
    if (ac.popoverPresentationController) { ac.popoverPresentationController.sourceView = self; ac.popoverPresentationController.sourceRect = self.bounds; }
    [top presentViewController:ac animated:YES completion:nil];
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

// ---- 1.12: apply action glass on first display and after reuse/state updates ----
static char kActionGlassKey;
static char kActionPendingKey;
static const NSInteger kActionGlassTag = 0x61A57;

static void TTGActionDebug(UIView *button) {
    static NSMutableSet<NSString *> *lines;
    static BOOL queued = NO, written = NO;
    if (written) return;
    if (!lines) lines = [NSMutableSet set];
    NSString *label = button.accessibilityLabel ?: @"";
    NSString *identifier = button.accessibilityIdentifier ?: @"";
    [lines addObject:[NSString stringWithFormat:@"%@ label=%@ id=%@", NSStringFromClass(button.class), label, identifier]];
    if (queued) return;
    queued = YES;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.8 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        written = YES;
        NSString *directory = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
        NSString *text = [@"Alaa_TikTok 1.12 matched action buttons\n" stringByAppendingString:
            [[[lines allObjects] sortedArrayUsingSelector:@selector(compare:)] componentsJoinedByString:@"\n"]];
        if (directory) [text writeToFile:[directory stringByAppendingPathComponent:@"alaa_tiktok_debug.txt"]
            atomically:YES encoding:NSUTF8StringEncoding error:nil];
        lines = nil;
    });
}

// TikTok has shipped both aweImageView and imageView for AWEFeedVideoButton.
// Only call known object-valued getters; never use unchecked private ivars/KVC.
static id TTGActionObject(id object, NSString *name) {
    SEL selector = NSSelectorFromString(name);
    if (!object || ![object respondsToSelector:selector]) return nil;
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

static BOOL TTGActionIdentity(UIView *button) {
    NSMutableString *identity = [NSMutableString string];
    for (NSString *key in @[@"imageNameString", @"accessibilityIdentifier", @"accessibilityLabel"]) {
        id value = TTGActionObject(button, key);
        if ([value isKindOfClass:NSString.class]) [identity appendFormat:@" %@", value];
    }
    NSString *name = [identity stringByFoldingWithOptions:NSDiacriticInsensitiveSearch locale:nil].lowercaseString;
    for (NSString *excluded in @[@"avatar", @"follow", @"music", @"disc", @"download", @"folgen", @"musik", @"متابعة", @"موسيقى"])
        if ([name containsString:excluded]) return NO;
    for (NSString *action in @[@"like", @"digg", @"heart", @"comment", @"collect", @"favorite", @"favourite", @"bookmark", @"save", @"share", @"forward", @"gefallt mir", @"kommentar", @"favoriten", @"teilen", @"speichern", @"اعجاب", @"تعليق", @"مفضلة", @"مشاركة", @"حفظ"])
        if ([name containsString:action]) return YES;
    // The confirmed action class may not populate its image-name getter until
    // highlight. A count within this exact button is a safe initial-display
    // fallback; TTGActionInFeed separately enforces the right-edge feed column.
    return TTGActionHasCount(button, 0);
}

static BOOL TTGActionInFeed(UIView *button) {
    UIWindow *window = button.window;
    if (!window || button.bounds.size.width < 24 || button.bounds.size.width > 100 ||
        button.bounds.size.height < 24 || button.bounds.size.height > 150) return NO;
    CGRect rect = [button convertRect:button.bounds toView:window];
    // Never decorate comments-panel/toolbars/profile actions.
    if (CGRectGetMidX(rect) < CGRectGetWidth(window.bounds) * 0.65 ||
        CGRectGetMidY(rect) < 80 || CGRectGetMidY(rect) > CGRectGetHeight(window.bounds) - 85) return NO;
    BOOL feed = NO;
    for (UIView *parent = button.superview; parent; parent = parent.superview) {
        NSString *name = NSStringFromClass(parent.class).lowercaseString;
        if ([parent isKindOfClass:UITabBar.class] || [name containsString:@"commentpanel"] ||
            [name containsString:@"commentsheet"] || [name containsString:@"profile"]) return NO;
        if ([name containsString:@"feed"] || [name containsString:@"playinteraction"]) feed = YES;
    }
    return feed;
}

// Some TikTok builds lazily fill imageView/aweImageView only on highlight.
// Inspect just this button's tiny icon subtree, never the full window hierarchy.
static UIView *TTGActionIcon(UIView *root, NSUInteger depth) {
    if (depth > 4) return nil;
    for (UIView *child in root.subviews) {
        if (child.tag == kActionGlassTag || child.hidden || child.alpha < 0.01 ||
            [child isKindOfClass:UIVisualEffectView.class]) continue;
        CGFloat width = child.bounds.size.width, height = child.bounds.size.height;
        if ([child isKindOfClass:UIImageView.class] && width >= 12 && width <= 60 && height >= 12 && height <= 60) return child;
        UIView *found = TTGActionIcon(child, depth + 1);
        if (found) return found;
    }
    return nil;
}


static char kGlyphIconKey, kGlyphOpacityKey, kGlyphImageKey, kGlyphSizeKey, kGlyphModeKey, kGlyphLikedKey;

// These effects are deliberately glyph-shaped, never circles or capsules.
static UIVisualEffect *TTGGlyphEffect(BOOL liked) {
    Class cls = NSClassFromString(@"UIGlassEffect");
    if (cls && [cls respondsToSelector:@selector(effectWithStyle:)]) {
        id effect = ((id (*)(id, SEL, NSInteger))objc_msgSend)(cls, @selector(effectWithStyle:), 1);
        if ([effect respondsToSelector:@selector(setTintColor:)])
            ((void (*)(id, SEL, id))objc_msgSend)(effect, @selector(setTintColor:),
                liked ? [UIColor colorWithRed:1 green:0.12 blue:0.24 alpha:0.30] : nil);
        return effect;
    }
    return [UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemThinMaterialDark];
}

static void TTGRestoreGlyph(UIView *button) {
    UIImageView *icon = objc_getAssociatedObject(button, &kGlyphIconKey);
    UIView *carrier = objc_getAssociatedObject(icon, &kActionGlassKey);
    NSNumber *opacity = objc_getAssociatedObject(icon, &kGlyphOpacityKey);
    if (opacity) icon.layer.opacity = opacity.floatValue;
    [carrier removeFromSuperview];
    objc_setAssociatedObject(icon, &kActionGlassKey, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    objc_setAssociatedObject(icon, &kGlyphOpacityKey, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    objc_setAssociatedObject(button, &kGlyphIconKey, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

// Rasterize only the template alpha for an inexpensive cached 0.65pt silhouette
// stroke. The glass itself remains a live native effect, masked by UIImageView.
static UIImage *TTGGlyphEdge(UIImage *image, CGSize size, UIViewContentMode mode) {
    if (!image || size.width <= 0 || size.height <= 0) return nil;
    UIImageView *shape = [[UIImageView alloc] initWithFrame:(CGRect){CGPointZero, size}];
    shape.image = [image imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];
    shape.tintColor = UIColor.whiteColor;
    shape.contentMode = mode;
    shape.clipsToBounds = YES;
    UIGraphicsBeginImageContextWithOptions(size, NO, 0);
    [shape.layer renderInContext:UIGraphicsGetCurrentContext()];
    UIImage *alpha = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    CGSize canvas = CGSizeMake(size.width + 2, size.height + 2);
    UIGraphicsBeginImageContextWithOptions(canvas, NO, 0);
    for (NSInteger dx = -1; dx <= 1; dx++) for (NSInteger dy = -1; dy <= 1; dy++) {
        if (dx || dy) [alpha drawInRect:CGRectMake(1 + dx * 0.65, 1 + dy * 0.65, size.width, size.height)];
    }
    [alpha drawInRect:CGRectMake(1, 1, size.width, size.height) blendMode:kCGBlendModeDestinationOut alpha:1];
    UIImage *edge = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return edge;
}

static void TTGApplyActionGlass(UIView *button) {
    if (!TTGEnabled() || !button.window || !TTGActionIdentity(button) || !TTGActionInFeed(button)) {
        TTGRestoreGlyph(button);
        return;
    }
    UIImageView *icon = nil;
    for (NSString *key in @[@"aweImageView", @"imageView"]) {
        id value = TTGActionObject(button, key);
        if ([value isKindOfClass:UIImageView.class] && value != button &&
            [value isDescendantOfView:button] && ![value isHidden] &&
            [value bounds].size.width >= 8 && [value bounds].size.height >= 8) { icon = value; break; }
    }
    if (!icon) icon = (UIImageView *)TTGActionIcon(button, 0);
    UIImage *image = (icon.highlighted && icon.highlightedImage) ? icon.highlightedImage : icon.image;
    if (!icon || !image || !icon.superview) return; // next settle pass retries lazy icon binding
    UIImageView *previous = objc_getAssociatedObject(button, &kGlyphIconKey);
    if (previous && previous != icon) TTGRestoreGlyph(button);
    objc_setAssociatedObject(button, &kGlyphIconKey, icon, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    UIView *carrier = objc_getAssociatedObject(icon, &kActionGlassKey);
    BOOL created = !carrier;
    if (!carrier) {
        carrier = [UIView new];
        carrier.tag = kActionGlassTag;
        carrier.userInteractionEnabled = NO;
        carrier.isAccessibilityElement = NO;
        carrier.accessibilityElementsHidden = YES;
        carrier.backgroundColor = UIColor.clearColor;
        UIVisualEffectView *effect = [[UIVisualEffectView alloc] initWithEffect:nil];
        effect.tag = 1;
        effect.userInteractionEnabled = NO;
        UIImageView *mask = [UIImageView new];
        mask.tintColor = UIColor.whiteColor;
        effect.maskView = mask;
        [carrier addSubview:effect];
        UIImageView *highlight = [UIImageView new];
        highlight.tag = 2;
        highlight.contentMode = UIViewContentModeScaleToFill;
        highlight.alpha = 0.42;
        [carrier addSubview:highlight];
        // Subtle inner light, not a replacement white solid glyph.
        UIImageView *light = [UIImageView new];
        light.tag = 3;
        light.alpha = 0.08;
        light.tintColor = UIColor.whiteColor;
        [carrier addSubview:light];
        objc_setAssociatedObject(icon, &kActionGlassKey, carrier, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        objc_setAssociatedObject(icon, &kGlyphOpacityKey, @(icon.layer.opacity), OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    if (carrier.superview != icon.superview) {
        [carrier removeFromSuperview];
        [icon.superview insertSubview:carrier aboveSubview:icon];
    }
    CGSize size = icon.bounds.size;
    CGRect bounds = CGRectMake(0, 0, size.width + 2, size.height + 2);
    if (!CGRectEqualToRect(carrier.bounds, bounds)) carrier.bounds = bounds;
    if (!CGPointEqualToPoint(carrier.center, icon.center)) carrier.center = icon.center;
    if (!CGAffineTransformEqualToTransform(carrier.transform, icon.transform)) carrier.transform = icon.transform;
    carrier.hidden = icon.hidden;
    carrier.alpha = icon.alpha;
    UIVisualEffectView *effect = (id)[carrier viewWithTag:1];
    UIImageView *edge = (id)[carrier viewWithTag:2], *light = (id)[carrier viewWithTag:3];
    UIImageView *mask = (id)effect.maskView;
    CGRect content = CGRectMake(1, 1, size.width, size.height);
    effect.frame = bounds;
    mask.frame = content;
    mask.contentMode = icon.contentMode;
    light.frame = content;
    light.contentMode = icon.contentMode;
    edge.frame = bounds;
    UIImage *oldImage = objc_getAssociatedObject(carrier, &kGlyphImageKey);
    NSValue *oldSize = objc_getAssociatedObject(carrier, &kGlyphSizeKey);
    NSNumber *oldMode = objc_getAssociatedObject(carrier, &kGlyphModeKey);
    if (created || image != oldImage || !CGSizeEqualToSize(oldSize.CGSizeValue, size) || oldMode.integerValue != icon.contentMode) {
        UIImage *template = [image imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];
        mask.image = template;
        light.image = template;
        edge.image = TTGGlyphEdge(image, size, icon.contentMode);
        objc_setAssociatedObject(carrier, &kGlyphImageKey, image, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        objc_setAssociatedObject(carrier, &kGlyphSizeKey, [NSValue valueWithCGSize:size], OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        objc_setAssociatedObject(carrier, &kGlyphModeKey, @(icon.contentMode), OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    id imageName = TTGActionObject(button, @"imageNameString");
    NSString *name = [imageName isKindOfClass:NSString.class] ? [imageName lowercaseString] : @"";
    BOOL heart = [name containsString:@"like"] || [name containsString:@"heart"] || [name containsString:@"digg"];
    BOOL selected = [button isKindOfClass:UIControl.class] && ((UIControl *)button).selected;
    BOOL liked = heart && (selected || [name containsString:@"after"] ||
        ([name containsString:@"selected"] && ![name containsString:@"unselected"]) ||
        ([name containsString:@"liked"] && ![name containsString:@"unliked"]));
    NSNumber *oldLiked = objc_getAssociatedObject(carrier, &kGlyphLikedKey);
    if (created || !oldLiked || oldLiked.boolValue != liked) {
        effect.effect = TTGGlyphEffect(liked);
        light.tintColor = liked ? [UIColor colorWithRed:1 green:0.15 blue:0.25 alpha:1] : UIColor.whiteColor;
        light.alpha = liked ? 0.18 : 0.08;
        objc_setAssociatedObject(carrier, &kGlyphLikedKey, @(liked), OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    // Keep UIKit's image/state intact for future reuse, but suppress solid paint.
    // Only its visual layer changes; original layout and hit areas are untouched.
    icon.layer.opacity = 0;
    TTGActionDebug(button);
}

// Attach/layout happen before TikTok finishes its lazy icon/model binding.
// A bounded next-turn + 120ms + 500ms settle pass covers first display without
// waiting for a touch. One batch per button, weak captures, no permanent timer.
static void TTGQueueActionGlass(UIView *button) {
    if (!button.window || [objc_getAssociatedObject(button, &kActionPendingKey) boolValue]) return;
    objc_setAssociatedObject(button, &kActionPendingKey, @YES, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    __weak UIView *weakButton = button;
    for (NSNumber *delay in @[@0.0, @0.12, @0.5]) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delay.doubleValue * NSEC_PER_SEC)),
            dispatch_get_main_queue(), ^{
                UIView *strongButton = weakButton;
                if (!strongButton) return;
                if (strongButton.window) {
                    [strongButton layoutIfNeeded];
                    TTGApplyActionGlass(strongButton);
                }
                if (delay.doubleValue >= 0.5)
                    objc_setAssociatedObject(strongButton, &kActionPendingKey, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
            });
    }
}

// Only traverse the current feed cell on display/model/layout callbacks.
// This is a small fallback for cells attaching preconfigured buttons together.
static void TTGPrepareFeedActions(UIView *root) {
    if (!root.window) return;
    Class actionClass = NSClassFromString(@"AWEFeedVideoButton");
    if (!actionClass) return;
    NSMutableArray<UIView *> *queue = [NSMutableArray arrayWithObject:root];
    for (NSUInteger i = 0; i < queue.count && i < 1000; i++) {
        UIView *view = queue[i];
        if (view.tag == kActionGlassTag || [view isKindOfClass:UIVisualEffectView.class]) continue;
        if ([view isKindOfClass:actionClass]) { TTGQueueActionGlass(view); continue; }
        if (queue.count < 1000) [queue addObjectsFromArray:view.subviews];
    }
}

%hook AWEFeedVideoButton
- (void)layoutSubviews {
    %orig;
    TTGApplyActionGlass((UIView *)self);
    TTGQueueActionGlass((UIView *)self);
}
- (void)didMoveToWindow {
    %orig;
    if (![(UIView *)self window]) {
        TTGRestoreGlyph((UIView *)self);
        return;
    }
    [(UIView *)self setNeedsLayout];
    TTGQueueActionGlass((UIView *)self);
}
- (void)didMoveToSuperview {
    %orig;
    TTGQueueActionGlass((UIView *)self);
}
- (void)setImageNameString:(NSString *)name {
    %orig;
    [(UIView *)self setNeedsLayout];
    TTGQueueActionGlass((UIView *)self);
}
- (void)setSelected:(BOOL)selected {
    %orig;
    [(UIView *)self setNeedsLayout];
    TTGQueueActionGlass((UIView *)self);
}
- (void)setHighlighted:(BOOL)highlighted {
    %orig;
    TTGQueueActionGlass((UIView *)self);
}
%end

%hook AWEFeedCellViewController
- (void)viewDidLayoutSubviews {
    %orig;
    UIViewController *controller = (UIViewController *)self;
    if (controller.isViewLoaded) TTGPrepareFeedActions(controller.view);
}
- (void)viewDidAppear:(BOOL)animated {
    %orig;
    UIViewController *controller = (UIViewController *)self;
    if (controller.isViewLoaded) TTGPrepareFeedActions(controller.view);
}
- (void)setModel:(id)model {
    %orig;
    UIViewController *controller = (UIViewController *)self;
    if (controller.isViewLoaded) TTGPrepareFeedActions(controller.view);
}
%end


// Track lazy/reused icon replacement without a window-wide traversal.
%hook UIImageView
- (void)setImage:(UIImage *)image {
    %orig;
    if (objc_getAssociatedObject(self, &kActionGlassKey)) {
        Class buttonClass = NSClassFromString(@"AWEFeedVideoButton");
        for (UIView *parent = [(UIView *)self superview]; parent; parent = parent.superview) {
            if (buttonClass && [parent isKindOfClass:buttonClass]) { TTGQueueActionGlass(parent); break; }
        }
    }
}
%end

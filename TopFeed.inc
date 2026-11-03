// Hide only compact, positively identified feed-header controls. Never hide the video/root.
static NSMapTable<UIView *,NSNumber *> *TT49TopHidden(void) {
    static NSMapTable *map;static dispatch_once_t once;
    dispatch_once(&once,^{map=[NSMapTable weakToStrongObjectsMapTable];});return map;
}
static void TT49RestoreTop(void) {
    NSMapTable *map=TT49TopHidden();
    for(UIView *view in map.keyEnumerator.allObjects)view.hidden=[[map objectForKey:view] boolValue];
    [map removeAllObjects];
}
static BOOL TT49TabText(NSString *s) {
    return [@[@"المجتمع",@"بالقرب مني",@"أتابعه",@"التسوق",@"لك",@"أتابع",@"For You",@"Following",@"Shop",@"Nearby",@"Friends"] containsObject:[s stringByTrimmingCharactersInSet:NSCharacterSet.whitespaceAndNewlineCharacterSet]];
}
static void TT49HideTop(UIWindow *window) {
    if(!window)return;
    NSMutableArray *queue=[NSMutableArray arrayWithObject:window];
    NSMutableArray<UIView *> *leaves=[NSMutableArray new];
    CGFloat limit=window.safeAreaInsets.top+120;
    for(NSUInteger i=0;i<queue.count && i<1800;i++) {
        UIView *v=queue[i];NSString *cls=NSStringFromClass(v.class);
        if([cls hasPrefix:@"TTJoker"] || [cls hasPrefix:@"TT122"] || [cls hasPrefix:@"TT46"] || [cls containsString:@"Keyboard"])continue;
        CGRect r=[v convertRect:v.bounds toView:window];
        if(r.origin.y<limit && CGRectGetMaxY(r)>0 && r.size.height<=120) {
            NSString *text=[v isKindOfClass:UILabel.class]?((UILabel *)v).text:nil;
            BOOL tab=TT49TabText(text);
            NSString *name=cls.lowercaseString;
            BOOL control=([name containsString:@"search"] || [name containsString:@"live"]) &&
                ([name containsString:@"button"] || [name containsString:@"entrance"] || [name containsString:@"icon"]);
            if((tab || control) && r.size.width>0 && !CGRectIsEmpty(r))[leaves addObject:v];
        }
        if(queue.count<2200)[queue addObjectsFromArray:v.subviews];
    }
    // Only proceed on the real feed header: at least two recognized tabs in its top strip.
    NSUInteger tabs=0;for(UIView *v in leaves)if([v isKindOfClass:UILabel.class] && TT49TabText(((UILabel *)v).text))tabs++;
    if(tabs<2)return;
    for(UIView *leaf in leaves) {
        UIView *target=leaf;
        // Collapse to a compact shared header ancestor when available, capturing icons too.
        for(UIView *p=leaf.superview;p && p!=window;p=p.superview) {
            CGRect r=[p convertRect:p.bounds toView:window];
            if(r.size.height>120 || r.origin.y>=limit || r.origin.y<0)break;
            if(r.size.width>window.bounds.size.width*.65 && r.size.height>=24)target=p;
        }
        if(![TT49TopHidden() objectForKey:target])[TT49TopHidden() setObject:@(target.hidden) forKey:target];
        target.hidden=YES;
    }
}

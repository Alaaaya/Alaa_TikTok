// File-scope fixture for Joker 65 UIKit tests. TEST DATA, not a device feed.
@interface JK65FixtureSource:NSObject<UITableViewDataSource>
@end
@implementation JK65FixtureSource
- (NSInteger)tableView:(UITableView *)t numberOfRowsInSection:(NSInteger)s{return 3;}
- (UITableViewCell *)tableView:(UITableView *)t cellForRowAtIndexPath:(NSIndexPath *)i{
    UITableViewCell *c=[t dequeueReusableCellWithIdentifier:@"c"]?:[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"c"];
    NSArray *cols=@[[UIColor colorWithRed:.85 green:.2 blue:.35 alpha:1],[UIColor colorWithRed:.15 green:.5 blue:.85 alpha:1],[UIColor colorWithRed:.2 green:.7 blue:.4 alpha:1]];
    c.contentView.backgroundColor=cols[i.row%3];c.backgroundColor=cols[i.row%3];
    c.textLabel.text=[NSString stringWithFormat:@"Video %ld · FIXTURE test data, not device",(long)i.row+1];
    c.textLabel.textColor=UIColor.whiteColor;c.textLabel.font=[UIFont boldSystemFontOfSize:20];
    return c;
}
@end

#import "UIColor+MGLAdditions.h"

@implementation UIColor (MGLAdditions)

- (mbgl::Color)mgl_color
{
    CGFloat r, g, b, a;
    
    [self getRed:&r green:&g blue:&b alpha:&a];
    for (NSNumber *num in @[@(r), @(g), @(b), @(a)]) {
        if ((MAX(MIN(num.floatValue, 1), 0)) != num.floatValue) {
            [NSException raise:NSInvalidArgumentException format:@""];
//            num = @0;
        }
    }
    return { (float)r, (float)g, (float)b, (float)a };
//    return { (float)[self cap_rgba:r], (float)[self cap_rgba:g], (float)[self cap_rgba:b], (float)[self cap_rgba:a] };
}

- (mbgl::style::PropertyValue<mbgl::Color>)mgl_colorPropertyValue
{
    mbgl::Color color = self.mgl_color;
    return {{ color.r, color.g, color.b, color.a }};
}

- (CGFloat)cap_rgba:(CGFloat)num
{
    return MAX(MIN(num, 1), 0);
}

+ (UIColor *)mgl_colorWithColor:(mbgl::Color)color
{
    return [UIColor colorWithRed:color.r green:color.g blue:color.b alpha:color.a];
}

@end

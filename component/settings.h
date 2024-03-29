#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

struct Interface {
    QString theme {};
    QString language {};
    QString date_format {};
    QString separator {};

    bool operator==(const Interface& other) const
    {
        return theme == other.theme && language == other.language && separator == other.separator && date_format == other.date_format;
    }

    bool operator!=(const Interface& other) const
    {
        return theme != other.theme || language != other.language || separator != other.separator || date_format != other.date_format;
    }
};

struct SectionRule {
    QString static_label {};
    int static_node {};
    QString dynamic_label {};
    int dynamic_node_lhs {};
    QString operation {};
    int dynamic_node_rhs {};
    bool hide_time {};
    int base_unit {};
    QString document_dir {};
    int value_decimal {};
    int rate_decimal {};

    bool operator==(const SectionRule& other) const
    {
        return static_label == other.static_label && static_node == other.static_node && dynamic_label == other.dynamic_label
            && dynamic_node_lhs == other.dynamic_node_lhs && dynamic_node_rhs == other.dynamic_node_rhs && operation == other.operation
            && hide_time == other.hide_time && base_unit == other.base_unit && document_dir == other.document_dir && value_decimal == other.value_decimal
            && rate_decimal == other.rate_decimal;
    }

    bool operator!=(const SectionRule& other) const
    {
        return static_label != other.static_label || static_node != other.static_node || dynamic_label != other.dynamic_label
            || dynamic_node_lhs != other.dynamic_node_lhs || dynamic_node_rhs != other.dynamic_node_rhs || operation != other.operation
            || hide_time != other.hide_time || base_unit != other.base_unit || document_dir != other.document_dir || value_decimal != other.value_decimal
            || rate_decimal != other.rate_decimal;
    }
};

#endif // SETTINGS_H

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QComboBox>
#include <QDialog>

#include "component/settings.h"
#include "component/using.h"

namespace Ui {
class Preferences;
}

class Preferences : public QDialog {
    Q_OBJECT

public:
    Preferences(CStringHash* currency_hash, CStringHash* leaf_path, CStringHash* branch_path, CStringList* date_format_list, const Interface& interface,
        const SectionRule& section_rule, QWidget* parent = nullptr);
    ~Preferences();

signals:
    void SUpdateSettings(const SectionRule& section_rule, const Interface& interface);

private slots:
    void on_pBtnApply_clicked();

    void on_comboDateTime_textActivated(const QString& arg1);

    void on_comboSeparator_textActivated(const QString& arg1);

    void on_comboBaseUnit_activated(int index);

    void on_spinValueDecimal_valueChanged(int arg1);

    void on_spinRateDecimal_valueChanged(int arg1);

    void on_comboTheme_textActivated(const QString& arg1);

    void on_comboLanguage_textActivated(const QString& arg1);

    void RCustomAccept();

    void on_pBtnDocumentDir_clicked();

    void on_pBtnResetDocumentDir_clicked();

    void on_lineStatic_textChanged(const QString& arg1);

    void on_comboStatic_activated(int index);

    void on_lineDynamic_textChanged(const QString& arg1);

    void on_comboDynamicLhs_activated(int index);

    void on_comboOperation_textActivated(const QString& arg1);

    void on_comboDynamicRhs_activated(int index);

    void on_checkHideTime_toggled(bool checked);

private:
    void IniDialog();
    void IniCombo(QComboBox* combo);
    void IniConnect();
    void IniStringList();

    void Data(CStringHash* unit_hash, CStringList* date_format_list);
    void IniComboData(QComboBox* combo, CStringHash* hash, int value);
    void IniComboData(QComboBox* combo, CStringHash* leaf, CStringHash* branch, int value);
    void IniComboData(QComboBox* combo, CStringList& list, CString& string);

private:
    Ui::Preferences* ui;

    QStringList theme_list_ {};
    QStringList language_list_ {};
    QStringList separator_list_ {};
    QStringList operation_list_ {};

    CStringHash* leaf_path_ {};
    CStringHash* branch_path_ {};

    Interface interface_ {};
    SectionRule section_rule_ {};
};

#endif // PREFERENCES_H

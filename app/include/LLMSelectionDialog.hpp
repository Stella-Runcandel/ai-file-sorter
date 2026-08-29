#ifndef LLMSELECTIONDIALOG_HPP
#define LLMSELECTIONDIALOG_HPP

#include "Types.hpp"

#include <QCoreApplication>
#include <QDialog>

#include <memory>
#include <string>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QWidget;

class Settings;

/**
 * @brief Dialog for configuring external AI endpoints (OpenAI, Gemini, Custom API).
 */
class LLMSelectionDialog : public QDialog
{
    Q_DECLARE_TR_FUNCTIONS(LLMSelectionDialog)
public:
    explicit LLMSelectionDialog(Settings& settings, QWidget* parent = nullptr);
    ~LLMSelectionDialog() override = default;

    LLMChoice get_selected_llm_choice() const;
    std::string get_selected_custom_api_id() const;
    std::string get_openai_api_key() const;
    std::string get_openai_model() const;
    std::string get_gemini_api_key() const;
    std::string get_gemini_model() const;

    // Legacy compatibility accessors returning safe defaults
    std::string get_selected_custom_llm_id() const { return {}; }
    bool get_llm_downloads_expanded() const { return false; }
    std::string get_llm_storage_dir() const { return {}; }
    std::string get_selected_visual_model_id() const { return {}; }

private:
    void setup_ui();
    void connect_signals();
    void update_ui_for_choice();
    void update_custom_api_buttons();
    void refresh_custom_api_lists();
    void select_custom_api_by_id(const std::string& id);
    void handle_add_custom_api();
    void handle_edit_custom_api();
    void handle_delete_custom_api();
    void validate_inputs();
    void accept() override;

    Settings& settings;
    LLMChoice selected_choice{LLMChoice::Remote_OpenAI};
    std::string selected_custom_api_id;

    std::string openai_api_key;
    std::string openai_model;
    std::string gemini_api_key;
    std::string gemini_model;

    QButtonGroup* provider_group{nullptr};
    QRadioButton* openai_radio{nullptr};
    QRadioButton* gemini_radio{nullptr};
    QRadioButton* custom_api_radio{nullptr};

    QWidget* openai_container{nullptr};
    QLineEdit* openai_api_key_edit{nullptr};
    QLineEdit* openai_model_edit{nullptr};
    QCheckBox* show_openai_api_key_checkbox{nullptr};

    QWidget* gemini_container{nullptr};
    QLineEdit* gemini_api_key_edit{nullptr};
    QLineEdit* gemini_model_edit{nullptr};
    QCheckBox* show_gemini_api_key_checkbox{nullptr};

    QWidget* custom_api_container{nullptr};
    QComboBox* custom_api_combo{nullptr};
    QPushButton* add_custom_api_button{nullptr};
    QPushButton* edit_custom_api_button{nullptr};
    QPushButton* delete_custom_api_button{nullptr};

    QPushButton* ok_button{nullptr};
    QPushButton* cancel_button{nullptr};
    QDialogButtonBox* button_box{nullptr};
};

#endif // LLMSELECTIONDIALOG_HPP

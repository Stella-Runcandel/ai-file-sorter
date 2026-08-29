#include "LLMSelectionDialog.hpp"

#include "AppIconResources.hpp"
#include "CustomApiDialog.hpp"
#include "DialogUtils.hpp"
#include "ErrorMessages.hpp"
#include "Settings.hpp"
#include "Utils.hpp"

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

LLMSelectionDialog::LLMSelectionDialog(Settings& settings, QWidget* parent)
    : QDialog(parent)
    , settings(settings)
{
    QIcon icon = QApplication::windowIcon();
    if (icon.isNull()) {
        icon = AppIconResources::build_window_icon();
    }
    if (icon.isNull()) {
        icon = QIcon(QStringLiteral(":/dev/hfstudio/AIFileSorter/images/logo.png"));
    }
    if (!icon.isNull()) {
        setWindowIcon(icon);
    }

    setWindowTitle(tr("AI Provider Settings"));
    setModal(true);
    setup_ui();
    connect_signals();

    openai_api_key = settings.get_openai_api_key();
    openai_model = settings.get_openai_model();
    if (openai_model.empty()) {
        openai_model = "gpt-4o-mini";
    }
    gemini_api_key = settings.get_gemini_api_key();
    gemini_model = settings.get_gemini_model();
    if (gemini_model.empty()) {
        gemini_model = "gemini-1.5-flash";
    }

    if (openai_api_key_edit) {
        openai_api_key_edit->setText(QString::fromStdString(openai_api_key));
    }
    if (openai_model_edit) {
        openai_model_edit->setText(QString::fromStdString(openai_model));
    }
    if (gemini_api_key_edit) {
        gemini_api_key_edit->setText(QString::fromStdString(gemini_api_key));
    }
    if (gemini_model_edit) {
        gemini_model_edit->setText(QString::fromStdString(gemini_model));
    }

    selected_choice = settings.get_llm_choice();
    selected_custom_api_id = settings.get_active_custom_api_id();

    switch (selected_choice) {
    case LLMChoice::Remote_Gemini:
        gemini_radio->setChecked(true);
        break;
    case LLMChoice::Remote_Custom:
        custom_api_radio->setChecked(true);
        break;
    case LLMChoice::Remote_OpenAI:
    default:
        selected_choice = LLMChoice::Remote_OpenAI;
        openai_radio->setChecked(true);
        break;
    }

    refresh_custom_api_lists();
    if (selected_choice == LLMChoice::Remote_Custom) {
        select_custom_api_by_id(selected_custom_api_id);
    }

    update_ui_for_choice();
    validate_inputs();

    setMinimumWidth(560);
    adjustSize();
}

void LLMSelectionDialog::setup_ui()
{
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(16);
    main_layout->setContentsMargins(20, 20, 20, 20);

    auto* header_label = new QLabel(
        tr("Configure the AI endpoint for categorization and multimodal file analysis."),
        this);
    header_label->setWordWrap(true);
    QFont header_font = header_label->font();
    header_font.setPointSizeF(header_font.pointSizeF() + 0.5);
    header_label->setFont(header_font);
    main_layout->addWidget(header_label);

    provider_group = new QButtonGroup(this);

    // --- OpenAI Group ---
    auto* openai_group = new QGroupBox(this);
    auto* openai_vbox = new QVBoxLayout(openai_group);
    openai_vbox->setSpacing(8);

    openai_radio = new QRadioButton(tr("OpenAI (Cloud Endpoint)"), openai_group);
    provider_group->addButton(openai_radio);
    openai_vbox->addWidget(openai_radio);

    openai_container = new QWidget(openai_group);
    auto* openai_form = new QFormLayout(openai_container);
    openai_form->setContentsMargins(24, 4, 4, 4);
    openai_form->setSpacing(8);

    openai_api_key_edit = new QLineEdit(openai_container);
    openai_api_key_edit->setEchoMode(QLineEdit::Password);
    openai_api_key_edit->setPlaceholderText(tr("Enter your OpenAI API key (sk-...)"));
    openai_form->addRow(tr("API Key:"), openai_api_key_edit);

    show_openai_api_key_checkbox = new QCheckBox(tr("Show API key"), openai_container);
    openai_form->addRow(QString(), show_openai_api_key_checkbox);

    openai_model_edit = new QLineEdit(openai_container);
    openai_model_edit->setPlaceholderText(QStringLiteral("gpt-4o-mini"));
    openai_form->addRow(tr("Model:"), openai_model_edit);

    auto* openai_help = new QLabel(
        tr("Need an API key? Get one from <a href=\"https://platform.openai.com/api-keys\">OpenAI Platform</a>."),
        openai_container);
    openai_help->setOpenExternalLinks(true);
    openai_form->addRow(QString(), openai_help);

    openai_vbox->addWidget(openai_container);
    main_layout->addWidget(openai_group);

    // --- Google Gemini Group ---
    auto* gemini_group = new QGroupBox(this);
    auto* gemini_vbox = new QVBoxLayout(gemini_group);
    gemini_vbox->setSpacing(8);

    gemini_radio = new QRadioButton(tr("Google Gemini (Cloud Endpoint)"), gemini_group);
    provider_group->addButton(gemini_radio);
    gemini_vbox->addWidget(gemini_radio);

    gemini_container = new QWidget(gemini_group);
    auto* gemini_form = new QFormLayout(gemini_container);
    gemini_form->setContentsMargins(24, 4, 4, 4);
    gemini_form->setSpacing(8);

    gemini_api_key_edit = new QLineEdit(gemini_container);
    gemini_api_key_edit->setEchoMode(QLineEdit::Password);
    gemini_api_key_edit->setPlaceholderText(tr("Enter your Gemini API key (AIza...)"));
    gemini_form->addRow(tr("API Key:"), gemini_api_key_edit);

    show_gemini_api_key_checkbox = new QCheckBox(tr("Show API key"), gemini_container);
    gemini_form->addRow(QString(), show_gemini_api_key_checkbox);

    gemini_model_edit = new QLineEdit(gemini_container);
    gemini_model_edit->setPlaceholderText(QStringLiteral("gemini-1.5-flash"));
    gemini_form->addRow(tr("Model:"), gemini_model_edit);

    auto* gemini_help = new QLabel(
        tr("Need an API key? Get one from <a href=\"https://aistudio.google.com/app/apikey\">Google AI Studio</a>."),
        gemini_container);
    gemini_help->setOpenExternalLinks(true);
    gemini_form->addRow(QString(), gemini_help);

    gemini_vbox->addWidget(gemini_container);
    main_layout->addWidget(gemini_group);

    // --- Custom OpenAI-Compatible API Group ---
    auto* custom_api_group = new QGroupBox(this);
    auto* custom_api_vbox = new QVBoxLayout(custom_api_group);
    custom_api_vbox->setSpacing(8);

    custom_api_radio = new QRadioButton(
        tr("Custom OpenAI-Compatible Endpoint (Local or Self-Hosted)"), custom_api_group);
    provider_group->addButton(custom_api_radio);
    custom_api_vbox->addWidget(custom_api_radio);

    custom_api_container = new QWidget(custom_api_group);
    auto* custom_api_layout = new QVBoxLayout(custom_api_container);
    custom_api_layout->setContentsMargins(24, 4, 4, 4);
    custom_api_layout->setSpacing(8);

    auto* custom_api_desc = new QLabel(
        tr("Connect to any OpenAI-compatible server (e.g. Ollama, LM Studio, LocalAI, vLLM, or corporate API)."),
        custom_api_container);
    custom_api_desc->setWordWrap(true);
    custom_api_layout->addWidget(custom_api_desc);

    auto* custom_api_controls = new QHBoxLayout();
    custom_api_controls->setSpacing(8);

    custom_api_combo = new QComboBox(custom_api_container);
    custom_api_combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    custom_api_controls->addWidget(custom_api_combo);

    add_custom_api_button = new QPushButton(tr("Add..."), custom_api_container);
    custom_api_controls->addWidget(add_custom_api_button);

    edit_custom_api_button = new QPushButton(tr("Edit..."), custom_api_container);
    custom_api_controls->addWidget(edit_custom_api_button);

    delete_custom_api_button = new QPushButton(tr("Delete"), custom_api_container);
    custom_api_controls->addWidget(delete_custom_api_button);

    custom_api_layout->addLayout(custom_api_controls);
    custom_api_vbox->addWidget(custom_api_container);
    main_layout->addWidget(custom_api_group);

    main_layout->addStretch(1);

    // --- Dialog Button Box ---
    button_box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    ok_button = button_box->button(QDialogButtonBox::Ok);
    cancel_button = button_box->button(QDialogButtonBox::Cancel);
    if (ok_button) {
        ok_button->setText(tr("Save"));
    }
    main_layout->addWidget(button_box);
}

void LLMSelectionDialog::connect_signals()
{
    connect(openai_radio, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            selected_choice = LLMChoice::Remote_OpenAI;
            update_ui_for_choice();
            validate_inputs();
        }
    });

    connect(gemini_radio, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            selected_choice = LLMChoice::Remote_Gemini;
            update_ui_for_choice();
            validate_inputs();
        }
    });

    connect(custom_api_radio, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            selected_choice = LLMChoice::Remote_Custom;
            update_ui_for_choice();
            validate_inputs();
        }
    });

    connect(show_openai_api_key_checkbox, &QCheckBox::toggled, this, [this](bool checked) {
        openai_api_key_edit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    connect(show_gemini_api_key_checkbox, &QCheckBox::toggled, this, [this](bool checked) {
        gemini_api_key_edit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    connect(openai_api_key_edit, &QLineEdit::textChanged, this, &LLMSelectionDialog::validate_inputs);
    connect(openai_model_edit, &QLineEdit::textChanged, this, &LLMSelectionDialog::validate_inputs);
    connect(gemini_api_key_edit, &QLineEdit::textChanged, this, &LLMSelectionDialog::validate_inputs);
    connect(gemini_model_edit, &QLineEdit::textChanged, this, &LLMSelectionDialog::validate_inputs);

    connect(custom_api_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index >= 0) {
            selected_custom_api_id = custom_api_combo->itemData(index).toString().toStdString();
        } else {
            selected_custom_api_id.clear();
        }
        update_custom_api_buttons();
        validate_inputs();
    });

    connect(add_custom_api_button, &QPushButton::clicked, this, &LLMSelectionDialog::handle_add_custom_api);
    connect(edit_custom_api_button, &QPushButton::clicked, this, &LLMSelectionDialog::handle_edit_custom_api);
    connect(delete_custom_api_button, &QPushButton::clicked, this, &LLMSelectionDialog::handle_delete_custom_api);

    connect(button_box, &QDialogButtonBox::accepted, this, &LLMSelectionDialog::accept);
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void LLMSelectionDialog::update_ui_for_choice()
{
    const bool is_openai = (selected_choice == LLMChoice::Remote_OpenAI);
    const bool is_gemini = (selected_choice == LLMChoice::Remote_Gemini);
    const bool is_custom_api = (selected_choice == LLMChoice::Remote_Custom);

    openai_container->setEnabled(is_openai);
    gemini_container->setEnabled(is_gemini);
    custom_api_container->setEnabled(is_custom_api);

    update_custom_api_buttons();
}

void LLMSelectionDialog::refresh_custom_api_lists()
{
    if (!custom_api_combo) {
        return;
    }

    custom_api_combo->blockSignals(true);
    custom_api_combo->clear();

    const auto endpoints = settings.get_custom_api_endpoints();
    for (const auto& endpoint : endpoints) {
        QString label = QString::fromStdString(endpoint.name);
        if (!endpoint.model.empty()) {
            label += QStringLiteral(" (%1)").arg(QString::fromStdString(endpoint.model));
        }
        custom_api_combo->addItem(label, QString::fromStdString(endpoint.id));
    }

    custom_api_combo->blockSignals(false);

    if (!selected_custom_api_id.empty()) {
        select_custom_api_by_id(selected_custom_api_id);
    } else if (custom_api_combo->count() > 0) {
        custom_api_combo->setCurrentIndex(0);
        selected_custom_api_id = custom_api_combo->itemData(0).toString().toStdString();
    }

    update_custom_api_buttons();
}

void LLMSelectionDialog::select_custom_api_by_id(const std::string& id)
{
    if (!custom_api_combo) {
        return;
    }
    const QString target_id = QString::fromStdString(id);
    for (int i = 0; i < custom_api_combo->count(); ++i) {
        if (custom_api_combo->itemData(i).toString() == target_id) {
            custom_api_combo->setCurrentIndex(i);
            selected_custom_api_id = id;
            update_custom_api_buttons();
            return;
        }
    }
}

void LLMSelectionDialog::update_custom_api_buttons()
{
    const bool has_selection = (custom_api_combo && custom_api_combo->currentIndex() >= 0);
    if (edit_custom_api_button) {
        edit_custom_api_button->setEnabled(has_selection);
    }
    if (delete_custom_api_button) {
        delete_custom_api_button->setEnabled(has_selection);
    }
}

void LLMSelectionDialog::handle_add_custom_api()
{
    CustomApiDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        const CustomApiEndpoint endpoint = dialog.result();
        const std::string id = settings.upsert_custom_api_endpoint(endpoint);
        selected_custom_api_id = id;
        refresh_custom_api_lists();
        select_custom_api_by_id(id);
        validate_inputs();
    }
}

void LLMSelectionDialog::handle_edit_custom_api()
{
    if (selected_custom_api_id.empty()) {
        return;
    }
    const CustomApiEndpoint existing = settings.find_custom_api_endpoint(selected_custom_api_id);
    if (existing.id.empty()) {
        return;
    }
    CustomApiDialog dialog(this, existing);
    if (dialog.exec() == QDialog::Accepted) {
        const CustomApiEndpoint updated = dialog.result();
        const std::string id = settings.upsert_custom_api_endpoint(updated);
        refresh_custom_api_lists();
        select_custom_api_by_id(id);
        validate_inputs();
    }
}

void LLMSelectionDialog::handle_delete_custom_api()
{
    if (selected_custom_api_id.empty()) {
        return;
    }
    const CustomApiEndpoint existing = settings.find_custom_api_endpoint(selected_custom_api_id);
    if (existing.id.empty()) {
        return;
    }

    const auto reply = QMessageBox::question(
        this,
        tr("Delete Custom Endpoint"),
        tr("Are you sure you want to delete the endpoint \"%1\"?").arg(QString::fromStdString(existing.name)),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        settings.remove_custom_api_endpoint(selected_custom_api_id);
        selected_custom_api_id.clear();
        refresh_custom_api_lists();
        validate_inputs();
    }
}

void LLMSelectionDialog::validate_inputs()
{
    if (!ok_button) {
        return;
    }

    bool valid = false;
    switch (selected_choice) {
    case LLMChoice::Remote_OpenAI: {
        const QString key = openai_api_key_edit ? openai_api_key_edit->text().trimmed() : QString();
        const QString model = openai_model_edit ? openai_model_edit->text().trimmed() : QString();
        valid = !key.isEmpty() && !model.isEmpty();
        break;
    }
    case LLMChoice::Remote_Gemini: {
        const QString key = gemini_api_key_edit ? gemini_api_key_edit->text().trimmed() : QString();
        const QString model = gemini_model_edit ? gemini_model_edit->text().trimmed() : QString();
        valid = !key.isEmpty() && !model.isEmpty();
        break;
    }
    case LLMChoice::Remote_Custom: {
        valid = !selected_custom_api_id.empty() && (custom_api_combo && custom_api_combo->currentIndex() >= 0);
        break;
    }
    default:
        valid = false;
        break;
    }

    ok_button->setEnabled(valid);
}

void LLMSelectionDialog::accept()
{
    if (openai_api_key_edit) {
        openai_api_key = openai_api_key_edit->text().trimmed().toStdString();
    }
    if (openai_model_edit) {
        openai_model = openai_model_edit->text().trimmed().toStdString();
    }
    if (gemini_api_key_edit) {
        gemini_api_key = gemini_api_key_edit->text().trimmed().toStdString();
    }
    if (gemini_model_edit) {
        gemini_model = gemini_model_edit->text().trimmed().toStdString();
    }

    if (selected_choice == LLMChoice::Remote_Custom && custom_api_combo && custom_api_combo->currentIndex() >= 0) {
        selected_custom_api_id = custom_api_combo->currentData().toString().toStdString();
    }

    QDialog::accept();
}

LLMChoice LLMSelectionDialog::get_selected_llm_choice() const
{
    return selected_choice;
}

std::string LLMSelectionDialog::get_selected_custom_api_id() const
{
    return selected_custom_api_id;
}

std::string LLMSelectionDialog::get_openai_api_key() const
{
    return openai_api_key;
}

std::string LLMSelectionDialog::get_openai_model() const
{
    return openai_model;
}

std::string LLMSelectionDialog::get_gemini_api_key() const
{
    return gemini_api_key;
}

std::string LLMSelectionDialog::get_gemini_model() const
{
    return gemini_model;
}

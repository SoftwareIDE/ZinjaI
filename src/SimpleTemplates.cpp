#include "SimpleTemplates.h"
#include "Cpp11.h"
#include "ConfigManager.h"
#include "mxUtils.h"

SimpleTemplates *g_templates = nullptr;

void SimpleTemplates::Initialize ( ) {
	if (!g_templates) g_templates = new SimpleTemplates();
}


SimpleTemplates::SimpleTemplates ( ) {
	wxString name_prefix = wxString("Name_")+config->Init.language_file;
	wxArrayString templates;
	mxUT::GetFilesFromBothDirs(templates,"templates",true);
	for (unsigned int i=0; i<templates.GetCount();i++) {
	
		TemplateInfo info;
		info.file_name = templates[i];
		
		wxString filename = mxUT::WichOne(info.file_name,"templates",true);
		std::map<wxString,wxString> opts; GetOptions(opts,filename,false);
		
		if (opts.count(name_prefix)) info.userf_name = opts[name_prefix];
		else if (opts.count("Name")) info.userf_name = opts["Name"];
		else info.userf_name = templates[i];
		
		info.cpp = opts.count("Type")==0 || opts["Type"]!="C";
		info.options = opts["Options"];
		
		m_templates.Add(info);
	}
}

wxString SimpleTemplates::GetNameFromFile (wxString file_name) {
	for(int i=0; i<m_templates.GetSize(); ++i)
		if (m_templates[i].file_name == file_name) 
			return m_templates[i].userf_name;
	return "";
}

void SimpleTemplates::GetFilesList (wxArrayString &list, bool c, bool cpp) {
	for(int i=0; i<m_templates.GetSize(); ++i)
		if ( m_templates[i].cpp?cpp:c ) 
			list.Add(m_templates[i].file_name);
}

void SimpleTemplates::GetNamesList (wxArrayString  &list, bool c, bool cpp) {
	for(int i=0; i<m_templates.GetSize(); ++i)
		if ( m_templates[i].cpp?cpp:c ) 
			list.Add(m_templates[i].userf_name);
}

wxString SimpleTemplates::GetParsedCompilerArgs (wxString file_name) {
	wxString options = "${DEFAULT}"; bool cpp = true;
	for(int i=0; i<m_templates.GetSize(); ++i)
		if (m_templates[i].file_name==file_name) {
			options = m_templates[i].options;
			cpp = m_templates[i].cpp;
		}
	options.Replace("${DEFAULT}",config->GetDefaultCompilerOptions(cpp),true);
	return options;
}

wxString SimpleTemplates::GetParsedCompilerArgs (bool for_cpp) {
	return GetParsedCompilerArgs(for_cpp?config->Files.cpp_template:config->Files.c_template);
}

int SimpleTemplates::GetOptions (map<wxString,wxString> &opts, wxString full_path, bool replace_default) {
	int headers_lines_count = 0;
	opts["Options"]="${DEFAULT}"; // ensure compiler args are never empty
	wxTextFile file(full_path);
	file.Open();
	if (file.IsOpened()) {
		wxString line = file.GetFirstLine();
		while (!file.Eof() && (line.Left(7)=="// !Z! "||line=="")) {
			++headers_lines_count;
			if (line.Contains(":")) {
				line = line.Mid(7); // strip the "// !Z! "
				opts[line.BeforeFirst(':')] = line.AfterFirst(':').Trim(true).Trim(false);
			}
			line=file.GetNextLine();
		}
		file.Close();
	}
	if (replace_default) {
		bool cpp = opts.count("Type")==0 || opts["Type"]!="C";
		opts["Options"].Replace("${DEFAULT}",config->GetDefaultCompilerOptions(cpp),true);
	}
	return headers_lines_count;
}

bool SimpleTemplates::IsCpp (wxString file_name) {
	for(int i=0; i<m_templates.GetSize(); ++i)
		if (m_templates[i].file_name==file_name)
			return m_templates[i].cpp;
	return true;
}


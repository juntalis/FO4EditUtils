{
	Change order priority for a bunch of constructible objects.
}
unit AddToArmorsmithScript;

const
	// AEC's Plugin FileName
	AECFileName = 'ArmorKeywords.esm';
	// The EditorID for the Armorsmith BNAM keyword
	AECArmorsmithKey = 'AEC_ck_ArmorsmithCraftingKey';
	// The EditorID for the Weaponsmith BNAM keyword
	AECWeaponsmithKey = 'AEC_ck_WeaponsmithCraftingKey';
	// The common EditorID prefix for AEC categories.
	AECCategoryPrefix = 'AEC_cm_';

var
	eCategory: IInterface;
	bWeaponWorkbench: boolean;
	sArmorKey, sWeaponKey: string;

function Initialize: integer;
var
	eAECFile : IInterface;
begin
	// default to success
	Result := 0;
	bWeaponWorkbench := false;
	
	// lookup AEC files
	eAECFile := FileByName(AECFileName, true);
	if not Assigned(eAECFile) then begin
		AddError('Could not locate AEC plugin! If the filename is no longer, '
		         '"' + AECFileName + '" edit this script and run again.');
		Result := 1;
		Exit;
	end;

	// Create our "Select Category" form.
	eCategory := SelectCategory(eAECFile);
	if not Assigned(eCategory) then begin
		Result := 2;
		Exit;
	end;
end;

function Process(e: IInterface): integer;
var
	sEditorID,sFormID: string;
	iFormID: cardinal;
	eCatKWD: IInterface;
	
begin
	Result := 0;
	sFormID := HexFormID(eCategory);
	iFormID := FormID(eCategory);
	sEditorID := EditorID(eCategory);
	if Signature(e) = 'COBJ' then begin
		AddMasterIfMissing(GetFile(e), AECFileName);
		eCatKWD := ElementByPath(e, 'BNAM - Workbench Keyword');
		if not Assigned(eCatKWD) then
			eCatKWD := ElementAssign(eCatKWD, LowInteger, nil, False);
		
		if bWeaponWorkbench then
			SetElementEditValues(e, 'BNAM - Workbench Keyword', sWeaponKey)
		else
			SetElementEditValues(e, 'BNAM - Workbench Keyword', sArmorKey);
		
		if not HasCategory(e, sEditorID) then begin
			eCatKWD := ElementByPath(e, 'FNAM - Category\Keyword');
			if not Assigned(eCatKWD) then begin
				eCatKWD := ElementByPath(e, 'FNAM - Category');
				eCatKWD := ElementAssign(eCatKWD, HighInteger, nil, False);
			end;
			SetEditValue(eCatKWD, sFormID);
		end;
	end;
	
end;

{
	FileByName:
	Gets a file from a filename.
	
	Example usage:
	f := FileByName('Fallout4.esm', true);
}
function FileByName(fileName: string; isESM: boolean = false): IInterface;
var
	i: integer;
	eFile: IInterface;
	sFileName: string;
begin
	Result := nil;
	for i := 0 to Pred(FileCount) do begin
		eFile := FileByIndex(i);
		sFileName := GetFileName(eFile);
		// if isESM is flagged, we only need to search until we hit
		// a non-ESM record. Since it defaults to false, we don't assume
		// non-ESMs when it's set to false.
		if isESM and not GetIsESM(eFile) then
			if Pos('.Hardcoded.', sFileName) = 0 then
				break;
		
		// Case-insensitive compare
		if SameText(sFileName, fileName) then begin
			Result := eFile;
			break;
		end;
	end;
end;

{
	HexFormID
	Gets the formID of a record as a hexadecimal string.
	
	This is useful for just about every time you want to deal with FormIDs.
	
	Example usage:
	s := HexFormID(e);
}
function HexFormID(e: IInterface): string;
var
	s: string;
begin
	s := GetElementEditValues(e, 'Record Header\FormID');
	if SameText(Signature(e), '') then 
		Result := '00000000'
	else  
		Result := Copy(s, Pos('[' + Signature(e) + ':', s) + Length(Signature(e)) + 2, 8);
end;

{
	HasKeyword:
	Checks if an input record has a keyword matching the input EditorID.
	
	Example usage:
	if HasKeyword(e, 'ArmorHeavy') then
		AddMessage(Name(e) + ' is a heavy armor.');
}
function HasCategory(e: IInterface; edid: string): boolean;
var
	fnam: IInterface;
	n: integer;
begin
	Result := false;
	fnam := ElementByPath(e, 'FNAM - Category');
	for n := 0 to ElementCount(fnam) - 1 do
		if GetElementEditValues(LinksTo(ElementByIndex(fnam, n)), 'EDID') = edid then 
			Result := true;
end;

{
	FullName
	Gets the "FULL - Name" element value for a record.
	
	Example usage:
	s := FullName(e);
}
function FullName(e: IInterface): string;
begin
	Result := GetElementEditValues(e, 'FULL - Name');
end;

{
	CheckedToBool:
	A function which returns a boolean corresponding to a 
	TCheckBoxState.
	
	Example usage:
	if CheckedToBool(cb.State) then
		AddMessage('Hi!');
}
function CheckedToBool(cbs: TCheckBoxState): boolean;
begin
	Result := (cbs = cbChecked);
end;

procedure AddError(error: string);
begin
	AddMessage('ERROR: ' + error);
end;

function SelectCategory(eAECFile: IInterface): IInterface;
var
	frm: TForm;
	lbl: TLabel;
	pnl: TPanel;
	cmb: TComboBox;
	chk: TCheckBox;
	sEditorID: string;
	i, iCount: integer;
	gKYWDs, eKYWD: IInterface;
begin
	Result := nil;
	sArmorKey := '';
	sWeaponKey := '';

	// Prompt user with "Select Category" form.
	frm := TForm.Create(nil);
	try
		frm.Caption := 'Select a Category';
		frm.Width := 380;
		frm.Height := 140;
		frm.Position := poScreenCenter;
		frm.BorderStyle := bsDialog;
		
		// create label instructing user what to do
		lbl := TLabel.Create(frm);
		lbl.Parent := frm;
		lbl.Left := 8;
		lbl.Top := 8;
		lbl.Width := frm.Width - 16;
		lbl.Caption := 'Select the category for your items:';
		
		// create panel to hold comboboxes
		pnl := TPanel.Create(frm);
		pnl.Parent := frm;
		pnl.Left := 8;
		pnl.Top := 32;
		pnl.Width := frm.Width - 16;
		pnl.Height := 30;
		pnl.BevelOuter := bvNone;
		
		// create Files combobox
		cmb := TComboBox.Create(frm);
		cmb.Parent := pnl;
		cmb.Left := 0;
		cmb.Top := 0;
		cmb.Width := 180;
		cmb.Autocomplete := True;
		cmb.Style := csDropDownList;
		cmb.Sorted := true;
		cmb.AutoDropDown := True;
		cmb.Text := '<Category>';
		
		
		chk := TCheckBox.create(frm);
		chk.Parent := pnl;
		chk.Left := cmb.Width + 16;
		chk.Top := 0;
		chk.Width := pnl.Width - chk.Left;
		chk.ShowHint := true;
		chk.Caption := 'Use Weaponsmith workbench';
		chk.Hint := 'Use Weaponsmith workbench instead of Armorsmith';
		
		// grab the keywords group and process the records
		gKYWDs := GroupBySignature(eAECFile, 'KYWD');
		iCount := ElementCount(gKYWDs);
		for i := 0 to Pred(iCount) do begin
			eKYWD := ElementByIndex(gKYWDs, i);
			sEditorID := EditorID(eKYWD);
			
			if sEditorID = AECArmorsmithKey then
				sArmorKey := HexFormID(eKYWD)
			else if sEditorID = AECWeaponsmithKey then
				sWeaponKey := HexFormID(eKYWD)
			else if Pos(AECCategoryPrefix, sEditorID) = 1 then
				cmb.Items.AddObject(FullName(eKYWD), eKYWD);
		end;
		
		cmb.ItemIndex := 0;
		
		
		if SameText(sArmorKey, '') or SameText(sWeaponKey, '') then begin
			AddError('Could not locate Armorsmith/Weaponsmith workbench keyword');
		end else begin
			// construct ok and cancel buttons
			cModal(frm, frm, 70);
			if frm.ShowModal = mrOk then begin
				bWeaponWorkbench := CheckedToBool(chk.State);
				if cmb.ItemIndex > -1 then begin
					Result := ObjectToElement(cmb.Items.Objects[cmb.ItemIndex]);
					AddMessage(Format('sArmorKey = %s, sWeaponKey = %s, sCategory = %s', [sArmorKey, sWeaponKey, Name(Result)]));
					//Result := ObjectToElement(cmb.Items.Objects[cmb.Items.IndexOf(cmb.Text)]);
				end;
			end;
		end;
	finally
		frm.Free;
	end;
end;


{
	ConstructModalButtons:
	A procedure which makes the standard OK and Cancel buttons 
	on a form.
	
	Example usage:
	ConstructModalButtons(frm, pnlBottom, frm.Height - 80);
}
procedure ConstructModalButtons(h, p: TObject; top: Integer);
var
	btnOk: TButton;
	btnCancel: TButton;
begin
	btnOk := TButton.Create(h);
	btnOk.Parent := p;
	btnOk.Caption := 'OK';
	btnOk.ModalResult := mrOk;
	btnOk.Left := h.Width div 2 - btnOk.Width - 8;
	btnOk.Top := top;
	
	btnCancel := TButton.Create(h);
	btnCancel.Parent := p;
	btnCancel.Caption := 'Cancel';
	btnCancel.ModalResult := mrCancel;
	btnCancel.Left := btnOk.Left + btnOk.Width + 16;
	btnCancel.Top := btnOk.Top;
end;

{
	cModal:
	Shortened function name for ConstructModalButtons.
}
procedure cModal(h, p: TObject; top: Integer);
begin
	ConstructModalButtons(h, p, top);
end;

end.
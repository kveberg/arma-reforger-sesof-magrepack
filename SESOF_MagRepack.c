// -------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------- SESOF_MagRepack -------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------------------------------------

//! When dragging and dropping items in the Inventory UI, the Action_Drop() function is called upon drop. It runs some checks before calling the 
//! MoveItemToStorage()-method. This mod patches in a repack-function there.
//! SESOF_MagRepack() performs checks to see if repacking should occur, and repacks.
// -------------------------------------------------------------------------------------------------------------------------------------------------------

modded class SCR_InventoryMenuUI
{	
	override void MoveItemToStorageSlot()
	{
		SESOF_MagRepack();
		super.MoveItemToStorageSlot();
	}
	
	void SESOF_MagRepack()
	{
		if (!m_pSelectedSlotUI)																					// are we dragging from an inventory slot?														
			return;

		if (m_pSelectedSlotUI.GetSlotedItemFunction() != ESlotFunction.TYPE_MAGAZINE)							// are we dragging a slot with an item that is a mag?
			return;
		
		if (m_pFocusedSlotUI.GetSlotedItemFunction() != ESlotFunction.TYPE_MAGAZINE)							// are we dropping on a slot with an item that is a mag?
			return;
		
																												
		InventoryItemComponent fromComp = m_pSelectedSlotUI.GetInventoryItemComponent();			
		InventoryItemComponent toComp = m_pFocusedSlotUI.GetInventoryItemComponent();
		//Print(fromComp);
		//Print(toComp);
		
		IEntity fromItem = fromComp.GetOwner();											
		IEntity toItem = toComp.GetOwner();				
		//Print(fromItem);
		//Print(toItem);
		
		if (!m_InventoryManager.CanMoveItem(fromItem))															// will the game allow us to move this item?
			return;
		if (!m_InventoryManager.CanMoveItem(toItem))											
			return;
		
		MagazineComponent fromMagComp = MagazineComponent.Cast(fromItem.FindComponent(MagazineComponent));		// get the magazine component of this item
		MagazineComponent toMagComp = MagazineComponent.Cast(toItem.FindComponent(MagazineComponent));	
		//Print(fromMagComp);
		//Print(toMagComp);
		
		if (fromMagComp.GetMagazineWell().Type() != toMagComp.GetMagazineWell().Type())							// are these compatible mags?
			return;

		if (fromMagComp.GetAmmoCount() == 0)																	// are there any bullets left in the mag we're dragging?
			return;
		
		if (toMagComp.GetAmmoCount() == toMagComp.GetMaxAmmoCount())											// is there room for any more bullets in the mag we're dropping on?
			return;
		
																												// well, then ... let's do some repacking!
		//Print("Available ammo:");
		//Print(fromMagComp.GetAmmoCount() + toMagComp.GetAmmoCount());
		//Print("From mag will receive:");
		//Print((fromMagComp.GetAmmoCount() + toMagComp.GetAmmoCount()) - toMagComp.GetMaxAmmoCount());
		//Print("To mag will receive:");
		//Print(toMagComp.GetMaxAmmoCount());
		
		int availableAmmo = fromMagComp.GetAmmoCount() + toMagComp.GetAmmoCount();
		
		if (availableAmmo > toMagComp.GetMaxAmmoCount())
		{
			fromMagComp.SetAmmoCount((fromMagComp.GetAmmoCount() + toMagComp.GetAmmoCount()) - toMagComp.GetMaxAmmoCount());
			toMagComp.SetAmmoCount(toMagComp.GetMaxAmmoCount());
		}
		else
			toMagComp.SetAmmoCount(availableAmmo);
			fromMagComp.SetAmmoCount(0);
		
		return;
	}
};
	

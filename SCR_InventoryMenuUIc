// -------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------- SESOF_MagRepack -------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------------------------------------
//! When dragging and dropping items in the Inventory UI, the Action_Drop()-method is called upon drop. It runs some checks before calling the 	     	 -
//! MoveItemToStorage()-method. This mod patches in a repack-function there.																			 -
//! SESOF_MagRepack() performs checks to see if repacking should occur, and repacks.																	 -
// -------------------------------------------------------------------------------------------------------------------------------------------------------

modded class SCR_InventoryMenuUI
{	
	override void MoveItemToStorageSlot()
	{
		MagRepack();
		super.MoveItemToStorageSlot();
		
	}
	
	void MagRepack()
	{
		Print("Trying to repack!");
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
		Print(fromItem);
		Print(toItem);
		
		if (fromItem.GetID() == toItem.GetID())																	// is from and to the same item?
		{	
			Print("Same mag!");
			return;
		}
		
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
		Print("Before repack ...");
		Print(toMagComp.GetAmmoCount());
		Print(fromMagComp.GetAmmoCount());
		
		private int availableAmmo = fromMagComp.GetAmmoCount() + toMagComp.GetAmmoCount();
		Print(availableAmmo);
		
		
		if (availableAmmo > toMagComp.GetMaxAmmoCount())
		{
			toMagComp.SetAmmoCount(toMagComp.GetMaxAmmoCount());
			fromMagComp.SetAmmoCount(availableAmmo - toMagComp.GetMaxAmmoCount());
			Print("After repack ...");
			Print(toMagComp.GetAmmoCount());
			Print(fromMagComp.GetAmmoCount());
			m_InventoryManager.PlayItemSound(toItem, SCR_SoundEvent.SOUND_PICK_UP);
			return;
		}
		
		
		if (availableAmmo <= toMagComp.GetMaxAmmoCount())
		{
			toMagComp.SetAmmoCount(availableAmmo);
			fromMagComp.SetAmmoCount(0);
			Print("After repack ...");
			Print(toMagComp.GetAmmoCount());
			Print(fromMagComp.GetAmmoCount());
			m_InventoryManager.PlayItemSound(toItem, SCR_SoundEvent.SOUND_PICK_UP);
			return;
		}
		

		Print("If you see this you're missing something and that is not good");
		return;
	}
};
	
